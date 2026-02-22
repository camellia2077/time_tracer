// utils/utils.cpp
#include "utils/utils.hpp"

#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace Utils {

// [核心修改] 移除 ConsoleColors 的静态成员定义

namespace {
constexpr int kLeapYearDivisor = 4;
constexpr int kCenturyDivisor = 100;
constexpr int kLeapCenturyDivisor = 400;
constexpr int kMinMonth = 1;
constexpr int kMaxMonth = 12;
constexpr int kFebruary = 2;
constexpr int kDaysInFebruaryLeap = 29;
constexpr int kDaysInFebruaryCommon = 28;
constexpr int kThirtyDayMonth = 30;
constexpr int kThirtyOneDayMonth = 31;
constexpr int kApril = 4;
constexpr int kJune = 6;
constexpr int kSeptember = 9;
constexpr int kNovember = 11;
}  // namespace

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
  return (year % kLeapYearDivisor == 0 && year % kCenturyDivisor != 0) ||
         (year % kLeapCenturyDivisor == 0);
}

auto get_days_in_month(const YearMonth& year_month) -> int {
  if (year_month.month < kMinMonth || year_month.month > kMaxMonth) {
    return 0;
  }
  if (year_month.month == kFebruary) {
    return is_leap(year_month.year) ? kDaysInFebruaryLeap
                                    : kDaysInFebruaryCommon;
  }
  if (year_month.month == kApril || year_month.month == kJune ||
      year_month.month == kSeptember || year_month.month == kNovember) {
    return kThirtyDayMonth;
  }
  return kThirtyOneDayMonth;
}

}  // namespace Utils
