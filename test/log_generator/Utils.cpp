#include "Utils.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace Utils {

    const std::string ConsoleColors::red = "\033[1;31m";
    const std::string ConsoleColors::green = "\033[1;32m";
    const std::string ConsoleColors::reset = "\033[0m";

    void setup_console() {
    #if defined(_WIN32) || defined(_WIN64)
        SetConsoleOutputCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
    #endif
    }
    
    bool is_leap(int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    int get_days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) {
            return is_leap(year) ? 29 : 28;
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            return 30;
        } else {
            return 31;
        }
    }

    void print_usage(const char* prog_name) {
        std::cerr << ConsoleColors::red << "Usage: " << ConsoleColors::reset << prog_name << " <start_year> <end_year> <items_per_day>\n";
        std::cerr << "       " << prog_name << " --version\n";
        std::cerr << "Description: Generates test log data for a given year range. Reads activities from 'activities_config.json'.\n";
        std::cerr << "  <start_year>      : The starting year (e.g., 1990).\n";
        std::cerr << "  <end_year>        : The ending year (inclusive).\n";
        std::cerr << "  <items_per_day>   : Number of log items per day (positive integer).\n";
        std::cerr << "  --version         : Display version information and exit.\n";
        std::cerr << "Example: " << prog_name << " 2023 2024 5\n";
    }

    void print_version() {
        const std::string APP_VERSION = "2.2.0";
        const std::string LAST_UPDATE = "2025-06-27";
        std::cout << "log_generator version " << APP_VERSION << std::endl;
        std::cout << "Last Updated: " << LAST_UPDATE << std::endl;
    }
} // namespace Utils