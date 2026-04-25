#include <vsomeip/vsomeip.hpp>
#include "auto_system.pb.h"
#include <iostream>
#include <memory>
#include <thread>
#include <condition_variable>

class TelemetryClient {
public:
    TelemetryClient() : app_(vsomeip::runtime::get()->create_application("TelemetryClient")), available_(false) {}

    void init() {
        app_->init();
        app_->register_state_handler([this](vsomeip::state_type_e s){
            if(s == vsomeip::state_type_e::ST_REGISTERED) app_->request_service(0x1234, 0x0001);
        });
        app_->register_availability_handler(0x1234, 0x0001, [this](vsomeip::service_t, vsomeip::instance_t, bool is_avail){
            std::lock_guard<std::mutex> lock(mu_);
            available_ = is_avail;
            cv_.notify_one();
        });
        app_->register_message_handler(0x1234, 0x0001, 0x0050, 
            std::bind(&TelemetryClient::on_response, this, std::placeholders::_1));
    }

    void start() { app_->start(); }

    void wait_and_send() {
        std::unique_lock<std::mutex> lock(mu_);
        cv_.wait(lock, [this]{ return available_; });
        
        auto req = vsomeip::runtime::get()->create_request();
        req->set_service(0x1234); req->set_instance(0x0001); req->set_method(0x0050);

        auto_system::TelemetryRequest proto_req;
        proto_req.set_vehicle_id(101);
        std::string out; proto_req.SerializeToString(&out);

        auto pl = vsomeip::runtime::get()->create_payload();
        pl->set_data(reinterpret_cast<const vsomeip::byte_t*>(out.data()), out.size());
        req->set_payload(pl);
        
        std::cout << "Client: Service discovered. Sending Request..." << std::endl;
        app_->send(req);
    }

    void on_response(const std::shared_ptr<vsomeip::message> &resp) {
        auto_system::VehicleStatus status;
        auto pl = resp->get_payload();
        if(status.ParseFromArray(pl->get_data(), pl->get_length())) {
            std::cout << "\n--- DATA RECEIVED ---" << std::endl;
            std::cout << "Speed: " << status.speed() << " km/h" << std::endl;
            std::cout << "Engine: " << status.engine_state() << std::endl;
            std::cout << "Fuel Level: " << (status.fuel_level() * 100) << "%" << std::endl;
            std::cout << "---------------------\n" << std::endl;
            app_->stop();
        }
    }

private:
    std::shared_ptr<vsomeip::application> app_;
    bool available_;
    std::mutex mu_;
    std::condition_variable cv_;
};

int main() {
    TelemetryClient client;
    client.init();
    std::thread t([&](){ 
        client.wait_and_send(); 
    });
    client.start();
    if(t.joinable()) t.join();
    return 0;
}
