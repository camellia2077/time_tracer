#pragma once
#include <string>

namespace Utils {
    struct ConsoleColors {
        static const std::string red;
        static const std::string green;
        static const std::string reset;
    };
    void setup_console();
    // REMOVED: std::string format_two_digits(int n);
    bool is_leap(int year);
    int get_days_in_month(int year, int month);
    void print_usage(const char* prog_name);
    void print_version();
}