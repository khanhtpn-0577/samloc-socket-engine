#pragma once
#include <string>
#include <mutex>

enum class LogLevel { INFO, DEBUG, WARNING, ERROR };

class Logger {
public:
    static void log(LogLevel level, const std::string& tag, const std::string& msg);
private:
    static std::mutex logMutex;
};