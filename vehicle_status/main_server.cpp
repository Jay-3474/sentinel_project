#include <vsomeip/vsomeip.hpp>
#include "auto_system.pb.h"
#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

class JobQueue {
public:
    void push(std::function<void()> j) {
        { std::lock_guard<std::mutex> lock(m_); jobs_.push(std::move(j)); }
        cv_.notify_one();
    }
    void run() {
        while(running_) {
            std::function<void()> j;
            {
                std::unique_lock<std::mutex> lock(m_);
                cv_.wait(lock, [this]{ return !jobs_.empty() || !running_; });
                if(!running_) break;
                j = std::move(jobs_.front()); jobs_.pop();
            }
            if(j) j();
        }
    }
    void stop() { { std::lock_guard<std::mutex> lock(m_); running_ = false; } cv_.notify_all(); }
private:
    std::queue<std::function<void()>> jobs_;
    std::mutex m_;
    std::condition_variable cv_;
    bool running_ = true;
};

class TelemetryServer {
public:
    TelemetryServer() : app_(vsomeip::runtime::get()->create_application("TelemetryServer")) {}

    void init() {
        app_->init();
        app_->register_message_handler(0x1234, 0x0001, 0x0050, 
            std::bind(&TelemetryServer::on_request, this, std::placeholders::_1));
        app_->offer_service(0x1234, 0x0001);
    }

    void start() {
        std::thread worker([this](){ queue_.run(); });
        app_->start(); 
        queue_.stop();
        if(worker.joinable()) worker.join();
    }

    void on_request(const std::shared_ptr<vsomeip::message> &req) {
        queue_.push([this, req]() {
            auto_system::TelemetryRequest proto_req;
            auto payload = req->get_payload();
            if (proto_req.ParseFromArray(payload->get_data(), payload->get_length())) {
                std::cout << "Server: Request for Vehicle ID " << proto_req.vehicle_id() << " received." << std::endl;
                
                auto resp = vsomeip::runtime::get()->create_response(req);
                auto_system::VehicleStatus status;
                status.set_speed(75.5f);
                status.set_engine_state("IDLE");
                status.set_fuel_level(0.65f); // 65% Fuel

                std::string serialized;
                status.SerializeToString(&serialized);
                auto resp_pl = vsomeip::runtime::get()->create_payload();
                resp_pl->set_data(reinterpret_cast<const vsomeip::byte_t*>(serialized.data()), serialized.size());
                resp->set_payload(resp_pl);
                
                std::cout << "Server: Sending Telemetry (Fuel: 65%) and stopping..." << std::endl;
                app_->send(resp);
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                app_->stop(); 
            }
        });
    }

private:
    std::shared_ptr<vsomeip::application> app_;
    JobQueue queue_;
};

int main() {
    TelemetryServer server;
    server.init();
    server.start();
    return 0;
}
