#pragma once
#include <string_view>

class LoggerInterface
{
    public:
        virtual void info(const std::string_view& message) = 0;
        virtual void warn(const std::string_view& message) = 0;
        virtual void error(const std::string_view& message) = 0;
};