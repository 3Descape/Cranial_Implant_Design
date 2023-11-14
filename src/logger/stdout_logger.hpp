#pragma once

#include <mutex>
#include <format>
#include <iostream>
#include <string_view>

#include "ansi.hpp"
#include "logger_interface.hpp"

class StdoutLogger : public LoggerInterface {
    private:
        std::mutex std_out_lock;

    public:
        StdoutLogger() = default;
        ~StdoutLogger() = default;
        StdoutLogger(const StdoutLogger&) = delete;
        StdoutLogger& operator=(const StdoutLogger&) = delete;

        void info(const std::string_view& message) override {
            std::lock_guard<std::mutex> lock_guard(std_out_lock);
            std::cout << ANSI_FG_CYAN << "[Info]    " << message << ANSI_RESET << std::endl;
        }

        void warn(const std::string_view& message) override {
            std::lock_guard<std::mutex> lock_guard(std_out_lock);
            std::cout << ANSI_FG_YELLOW << "[WARNING] " << message << ANSI_RESET << std::endl;
        }

        void error(const std::string_view& message) override {
            std::lock_guard<std::mutex> lock_guard(std_out_lock);
            std::cout << ANSI_FG_RED << "[ERROR]   " << message << ANSI_RESET << std::endl;
        }
};