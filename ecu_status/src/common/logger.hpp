#pragma once
#include <iostream>
#include <mutex>
#include <string>
#include <chrono>

enum class LogLevel { INFO, WARN, ERROR };

class Logger {
public:
    static void log(LogLevel level, const std::string& app, const std::string& msg) {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buf[10];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now));
        std::string lvl = (level == LogLevel::INFO) ? "INFO" : (level == LogLevel::WARN ? "WARN" : "ERR ");
        std::cout << "[" << buf << "] [" << lvl << "] [" << app << "] " << msg << std::endl;
    }
};
