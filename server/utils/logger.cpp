#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

std::mutex Logger::logMutex;

void Logger::log(LogLevel level, const std::string& tag, const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMutex);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::cout << "[" << std::put_time(std::localtime(&now_c), "%H:%M:%S") 
              << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";

    switch (level) {
        case LogLevel::INFO:    std::cout << "\033[32m[INFO]\033[0m "; break; // Green
        case LogLevel::DEBUG:   std::cout << "\033[34m[DEBUG]\033[0m "; break; // Blue
        case LogLevel::WARNING: std::cout << "\033[33m[WARN]\033[0m "; break; // Yellow
        case LogLevel::ERROR:   std::cout << "\033[31m[ERROR]\033[0m "; break; // Red
    }

    std::cout << "[" << tag << "] " << msg << std::endl;
}