// utils/utils.cpp
#include "utils/utils.hpp"

#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace Utils {

// [核心修改] 移除 ConsoleColors 的静态成员定义

void setup_console() {
#if defined(_WIN32) || defined(_WIN64)
  SetConsoleOutputCP(CP_UTF8);
  HANDLE h_out = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h_out != INVALID_HANDLE_VALUE) {
    DWORD dw_mode = 0;
    if (GetConsoleMode(h_out, &dw_mode) != 0) {
      dw_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(h_out, dw_mode);
    }
  }
#endif
}

auto is_leap(int year) -> bool {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

auto get_days_in_month(int year, int month) -> int {
  if (month < 1 || month > 12) {
    return 0;
  }
  if (month == 2) {
    return is_leap(year) ? 29 : 28;
  }
  if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  } else {
    return 31;
  }
}

}  // namespace Utils
