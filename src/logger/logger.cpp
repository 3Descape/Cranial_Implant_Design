#include <string>

#include "logger.hpp"

void report_assertion_failure(const char* expression, const char* message, const char* file, uint32_t line) {
    logger->error("Assertion Failure: " + std::string(expression) + ", message: '" + message + "', in file: " + file + ", line: " + std::to_string(line));
}