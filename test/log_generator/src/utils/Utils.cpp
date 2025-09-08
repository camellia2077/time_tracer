#include "utils/Utils.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace Utils {

    // [核心修改] 移除 ConsoleColors 的静态成员定义

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

} // namespace Utils