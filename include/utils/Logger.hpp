#pragma once

#include <fmt/core.h>
#include <fmt/chrono.h>
#include <string>
#include <chrono>
#include <iostream>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void init() {
        // Initialize logging system if needed
    }

    template<typename... Args>
    static void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::INFO, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void warning(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::WARNING, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(Level::ERROR, fmt, std::forward<Args>(args)...);
    }

private:
    template<typename... Args>
    static void log(Level level, fmt::format_string<Args...> fmt, Args&&... args) {
        auto now = std::chrono::system_clock::now();
        auto message = fmt::format(fmt, std::forward<Args>(args)...);
        auto levelStr = getLevelString(level);
        
        std::cout << fmt::format("[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n",
                                now, levelStr, message);
    }

    static const char* getLevelString(Level level) {
        switch (level) {
            case Level::DEBUG: return "DEBUG";
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
}; 