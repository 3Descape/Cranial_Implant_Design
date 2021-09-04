#include <chrono>
#include <string>
#include "util_string.hpp"

#ifndef UTIL_TIMER_H
#define UTIL_TIMER_H

class Timer
{
    std::chrono::steady_clock::time_point t0;
    std::chrono::steady_clock::time_point t1;
    double timestamp = 0.0;
public:
    Timer() { start(); }
    void start() { t0 = std::chrono::steady_clock::now(); }

    const Timer& stop()
    {
        t1 = std::chrono::steady_clock::now();
        return *this;
    }

    inline float milliseconds() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }

    inline float seconds() const
    {
        return milliseconds() / 1000.0f;
    }

    inline float minutes() const
    {
        return seconds() / 60.0f;
    }

    std::string toString() const
    {
        float duration = milliseconds();
        if(duration < 1000.0f)
            return to_string_with_precision(duration, 2) + " milliseconds";
        else if(duration < 60000.0f)
            return to_string_with_precision(seconds(), 2) + " seconds";

        return to_string_with_precision(minutes(), 2) + " minutes";
    }
};

#endif // UTIL_TIMER_H