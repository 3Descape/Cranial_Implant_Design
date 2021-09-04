#include <string>
#include <vector>
#include <sstream>

#ifndef UTIL_STRING_H
#define UTIL_STRING_H

inline std::vector<std::string> split_by_character(const std::string str, const char character)
{
    std::vector<std::string> words;

    std::string word = "";
    for (auto x : str)
    {
        if (x == character)
        {
            words.push_back(word);
            word = "";
        }
        else
        {
            word = word + x;
        }
    }

    return words;
}

inline std::vector<std::string> split_by_spaces(const std::string str)
{
    return split_by_character(str, ' ');
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

#endif