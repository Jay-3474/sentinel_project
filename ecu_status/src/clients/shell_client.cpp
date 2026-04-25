#include "vshield_common.hpp"
#include "logger.hpp"
#include "auto_system.pb.h"
#include <thread>
#include <csignal>

std::shared_ptr<vsomeip::application> g_app;

void shut(int) { if(g_app) g_app->stop(); }

int main() {
    std::signal(SIGINT, shut);
    g_app = vsomeip::runtime::get()->create_application("ShellClient");
    g_app->init();

    // Handler for the StatusResponse message
    g_app->register_message_handler(sentinel::SERVICE_ID, sentinel::INSTANCE_ID, sentinel::STATUS_METHOD,
        [](const std::shared_ptr<vsomeip::message>& msg) {
            vshield::StatusResponse res;
            if (res.ParseFromArray(msg->get_payload()->get_data(), msg->get_payload()->get_length())) {
                std::cout << "\n[RESULT] Packets: " << res.total_packets() 
                          << " | ECUs: " << res.active_ecus() 
                          << " | Mode: " << res.system_mode() << std::endl;
                std::cout << "SHELL> " << std::flush;
            }
        });

    g_app->register_availability_handler(sentinel::SERVICE_ID, sentinel::INSTANCE_ID,
        [](vsomeip::service_t, vsomeip::instance_t, bool avail) {
            if (avail) g_app->request_service(sentinel::SERVICE_ID, sentinel::INSTANCE_ID);
        });

    std::thread cli([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::string cmd;
        while (true) {
            std::cout << "SHELL> " << std::flush;
            if(!(std::cin >> cmd) || cmd == "exit") { g_app->stop(); break; }

            if (cmd == "status") {
                auto req = vsomeip::runtime::get()->create_request();
                req->set_service(sentinel::SERVICE_ID);
                req->set_instance(sentinel::INSTANCE_ID);
                req->set_method(sentinel::STATUS_METHOD); // Use dedicated status method

                auto p = vsomeip::runtime::get()->create_payload(); // Empty payload for status trigger
                req->set_payload(p);
                g_app->send(req);
            }
        }
    });

    g_app->start();
    if (cli.joinable()) cli.join();
    return 0;
}
