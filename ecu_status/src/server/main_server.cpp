#include "vshield_common.hpp"
#include "logger.hpp"
#include "auto_system.pb.h"
#include <queue>
#include <mutex>
#include <future>
#include <csignal>
#include <atomic>
#include <set>

std::shared_ptr<vsomeip::application> g_app;
std::atomic<bool> g_running(true);

void handle_signal(int) { 
    g_running = false; 
    if (g_app) g_app->stop(); 
}

class SentinelServer {
public:
    SentinelServer(std::shared_ptr<vsomeip::application> app) 
        : app_(app), packet_count_(0) {}

    void init() {
        app_->init();
        
        // Register Telemetry Handler
        app_->register_message_handler(sentinel::SERVICE_ID, sentinel::INSTANCE_ID, sentinel::TELEMETRY_METHOD,
            std::bind(&SentinelServer::on_telemetry, this, std::placeholders::_1));

        // Register Status Handler
        app_->register_message_handler(sentinel::SERVICE_ID, sentinel::INSTANCE_ID, sentinel::STATUS_METHOD,
            std::bind(&SentinelServer::on_status_request, this, std::placeholders::_1));

        app_->offer_service(sentinel::SERVICE_ID, sentinel::INSTANCE_ID);
        Logger::log(LogLevel::INFO, "Gateway", "Gateway offering services...");
    }

    void on_telemetry(const std::shared_ptr<vsomeip::message>& msg) {
        vshield::Telemetry data;
        data.ParseFromArray(msg->get_payload()->get_data(), msg->get_payload()->get_length());
        
        packet_count_++;
        active_ecus_.insert(data.ecu_id());
        
        Logger::log(LogLevel::INFO, "Gateway", "ECU_" + std::to_string(data.ecu_id()) + " value: " + std::to_string(data.value()));
        
        auto resp = vsomeip::runtime::get()->create_response(msg);
        app_->send(resp);
    }

    void on_status_request(const std::shared_ptr<vsomeip::message>& msg) {
        vshield::StatusResponse res;
        res.set_total_packets(packet_count_);
        res.set_active_ecus(active_ecus_.size());
        res.set_system_mode("V-SHIELD-ACTIVE");

        std::string out;
        res.SerializeToString(&out);
        
        auto resp = vsomeip::runtime::get()->create_response(msg);
        auto p = vsomeip::runtime::get()->create_payload();
        p->set_data(reinterpret_cast<const vsomeip::byte_t*>(out.data()), out.size());
        resp->set_payload(p);
        
        app_->send(resp);
        Logger::log(LogLevel::WARN, "Gateway", "Status report sent to Shell Client.");
    }

private:
    std::shared_ptr<vsomeip::application> app_;
    uint32_t packet_count_;
    std::set<uint32_t> active_ecus_;
};

int main() {
    std::signal(SIGINT, handle_signal);
    g_app = vsomeip::runtime::get()->create_application("Gateway");
    SentinelServer server(g_app);
    server.init();
    g_app->start();
    return 0;
}
