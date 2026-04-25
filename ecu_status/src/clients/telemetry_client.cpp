#include "vshield_common.hpp"
#include "logger.hpp"
#include "auto_system.pb.h"
#include <thread>
#include <csignal>

std::shared_ptr<vsomeip::application> g_app;
std::atomic<bool> g_run(true);

void shut(int) { g_run = false; if(g_app) g_app->stop(); }

int main() {
    std::signal(SIGINT, shut);
    g_app = vsomeip::runtime::get()->create_application("TelemetryClient");
    g_app->init();

    g_app->register_availability_handler(sentinel::SERVICE_ID, sentinel::INSTANCE_ID,
        [](vsomeip::service_t, vsomeip::instance_t, bool avail) {
            if (avail) g_app->request_service(sentinel::SERVICE_ID, sentinel::INSTANCE_ID);
        });

    std::thread t([]() {
        while (g_run) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if (!g_run) break;

            auto req = vsomeip::runtime::get()->create_request();
            req->set_service(sentinel::SERVICE_ID);
            req->set_instance(sentinel::INSTANCE_ID);
            req->set_method(sentinel::TELEMETRY_METHOD);

            vshield::Telemetry d;
            d.set_ecu_id(101);
            d.set_value(static_cast<float>(rand() % 100));
            d.set_timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            d.set_status("OPERATIONAL");

            std::string s; d.SerializeToString(&s);
            auto p = vsomeip::runtime::get()->create_payload();
            p->set_data(reinterpret_cast<const vsomeip::byte_t*>(s.data()), s.size());
            req->set_payload(p);
            g_app->send(req);
            Logger::log(LogLevel::INFO, "Telemetry", "Packet sent.");
        }
    });

    g_app->start();
    if (t.joinable()) t.join();
    return 0;
}
