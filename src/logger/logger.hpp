#pragma once

#include <memory>
#include <format>
#include <string_view>
#include "assert.hpp"
#include "logger_interface.hpp"

extern std::shared_ptr<LoggerInterface> logger;

// std::format reference https://en.cppreference.com/w/cpp/utility/format/formatter
template <typename... Args>
void log_info(const std::string_view& message, Args... arguments) {
    logger->info(std::vformat(message, std::make_format_args(arguments...)));
}

template <typename... Args>
void log_warn(const std::string_view& message, Args... arguments) {
    logger->warn(std::vformat(message, std::make_format_args(arguments...)));
}

template <typename... Args>
void log_error(const std::string_view& message, Args... arguments) {
    logger->error(std::vformat(message, std::make_format_args(arguments...)));
    #ifdef DEBUG
    debugBreak();
    #endif
}

enum Log_Level {
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
};

#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

#define LOG_ERROR(message, ...) log_error(message, __VA_ARGS__)
#define LOG_ERROR_ENABLED

#ifdef DEBUG
#if LOG_LEVEL == 0
#define LOG_INFO(message, ...)
#define LOG_WARN(message, ...)
#endif

#if LOG_LEVEL <= 1
#define LOG_INFO(message, ...)
#endif

#if LOG_LEVEL >= 1
#define LOG_WARN(message, ...) log_warn(message, __VA_ARGS__)
#define LOG_WARN_ENABLED
#endif

#if LOG_LEVEL >= 2
#define LOG_INFO(message, ...) log_info(message, __VA_ARGS__)
#define LOG_INFO_ENABLED
#endif
#endif
