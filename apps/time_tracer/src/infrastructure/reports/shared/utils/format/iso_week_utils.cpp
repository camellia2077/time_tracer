// infrastructure/reports/shared/utils/format/iso_week_utils.cpp
#include "infrastructure/reports/shared/utils/format/iso_week_utils.hpp"

#include <chrono>
#include <optional>
#include <string>

namespace {
using std::chrono::days;
using std::chrono::January;
using std::chrono::sys_days;
using std::chrono::weekday;
using std::chrono::year;
using std::chrono::year_month_day;

namespace {
constexpr int kDateStringLength = 10;
constexpr int kDashPosition1 = 4;
constexpr int kDashPosition2 = 7;
constexpr int kYearLength = 4;
constexpr int kMonthLength = 2;
constexpr int kDayLength = 2;
constexpr int kIsoWeekLength8 = 8;
constexpr int kIsoWeekLength7 = 7;
constexpr int kMaxWeeksInYear = 53;
constexpr int kMonthOffset = 5;
constexpr int kIsoWeekNumberOffset = 6;
constexpr int kDayOffset = 8;
constexpr int kDecimalBase = 10;
constexpr int kDaysInWeek = 7;
constexpr int kThursdayOffset = 4;
constexpr int kJan4Day = 4;
constexpr int kFirstMondayOffset = 3;
}  // namespace

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void AppendPaddedNumber(std::string& output, int value, int width) {
  std::string digits = std::to_string(value);
  if (digits.size() < static_cast<size_t>(width)) {
    output.append(static_cast<size_t>(width) - digits.size(), '0');
  }
  output += digits;
}

auto ParseDate(std::string_view date_str) -> std::optional<sys_days> {
  if (date_str.size() != static_cast<size_t>(kDateStringLength) ||
      date_str[kDashPosition1] != '-' || date_str[kDashPosition2] != '-') {
    return std::nullopt;
  }

  auto to_int = [](std::string_view input_sv) -> int {
    int value = 0;
    for (char digit_char : input_sv) {
      if (digit_char < '0' || digit_char > '9') {
        return -1;
      }
      value = (value * kDecimalBase) + (digit_char - '0');
    }
    return value;
  };

  int year_val = to_int(date_str.substr(0, kYearLength));
  int month_val = to_int(date_str.substr(kMonthOffset, kMonthLength));
  int day_val = to_int(date_str.substr(kDayOffset, kDayLength));

  if (year_val < 0 || month_val <= 0 || day_val <= 0) {
    return std::nullopt;
  }

  year_month_day ymd = year{year_val} /
                       std::chrono::month{static_cast<unsigned>(month_val)} /
                       std::chrono::day{static_cast<unsigned>(day_val)};
  if (!ymd.ok()) {
    return std::nullopt;
  }
  return sys_days{ymd};
}

auto FormatDate(const sys_days& date) -> std::string {
  year_month_day ymd = date;
  std::string output;
  output.reserve(kDateStringLength);
  AppendPaddedNumber(output, int(ymd.year()), 4);
  output += "-";
  AppendPaddedNumber(output, static_cast<int>(unsigned(ymd.month())), 2);
  output += "-";
  AppendPaddedNumber(output, static_cast<int>(unsigned(ymd.day())), 2);
  return output;
}
}  // namespace

auto ParseIsoWeek(std::string_view input, IsoWeek& out) -> bool {
  if (input.size() != static_cast<size_t>(kIsoWeekLength8) &&
      input.size() != static_cast<size_t>(kIsoWeekLength7)) {
    return false;
  }

  int year_val = 0;
  int week_val = 0;

  auto to_int = [](std::string_view input_sv) -> int {
    int value = 0;
    for (char digit_char : input_sv) {
      if (digit_char < '0' || digit_char > '9') {
        return -1;
      }
      value = (value * kDecimalBase) + (digit_char - '0');
    }
    return value;
  };

  if (input.size() == static_cast<size_t>(kIsoWeekLength8)) {
    if (input[kDashPosition1] != '-' ||
        (input[kMonthOffset] != 'W' && input[kMonthOffset] != 'w')) {
      return false;
    }
    year_val = to_int(input.substr(0, kYearLength));
    week_val = to_int(input.substr(kIsoWeekNumberOffset, kMonthLength));
  } else {
    if (input[kDashPosition1] != 'W' && input[kDashPosition1] != 'w') {
      return false;
    }
    year_val = to_int(input.substr(0, kYearLength));
    week_val = to_int(input.substr(kMonthOffset, 2));
  }

  if (year_val <= 0 || week_val < 1 || week_val > kMaxWeeksInYear) {
    return false;
  }

  IsoWeek candidate{.year = year_val, .week = week_val};
  std::string start_date = IsoWeekStartDate(candidate);
  IsoWeek validated = IsoWeekFromDate(start_date);
  if (validated.year != candidate.year || validated.week != candidate.week) {
    return false;
  }

  out = candidate;
  return true;
}

auto FormatIsoWeek(const IsoWeek& week) -> std::string {
  std::string output;
  output.reserve(static_cast<size_t>(kIsoWeekLength8));
  AppendPaddedNumber(output, week.year, 4);
  output += "-W";
  AppendPaddedNumber(output, week.week, 2);
  return output;
}

auto IsoWeekFromDate(std::string_view date_str) -> IsoWeek {
  auto date_opt = ParseDate(date_str);
  if (!date_opt) {
    return {};
  }

  sys_days date = *date_opt;
  int iso_weekday =
      static_cast<int>(weekday{date}.iso_encoding());  // 1=Mon ... 7=Sun
  sys_days thursday = date + days{kThursdayOffset - iso_weekday};
  year_month_day thursday_ymd = thursday;
  int iso_year = int(thursday_ymd.year());

  sys_days jan4 = sys_days{year{iso_year} / January / kJan4Day};
  int jan4_weekday = static_cast<int>(weekday{jan4}.iso_encoding());
  sys_days first_thursday = jan4 + days{kThursdayOffset - jan4_weekday};

  int week =
      static_cast<int>((thursday - first_thursday).count() / kDaysInWeek) + 1;

  return {.year = iso_year, .week = week};
}

auto IsoWeekStartDate(const IsoWeek& week) -> std::string {
  sys_days jan4 = sys_days{year{week.year} / January / kJan4Day};
  int jan4_weekday = static_cast<int>(weekday{jan4}.iso_encoding());
  sys_days first_thursday = jan4 + days{kThursdayOffset - jan4_weekday};
  sys_days first_monday = first_thursday - days{kFirstMondayOffset};
  sys_days start = first_monday +
                   days{static_cast<long long>(kDaysInWeek) * (week.week - 1)};
  return FormatDate(start);
}

auto IsoWeekEndDate(const IsoWeek& week) -> std::string {
  auto start_date = ParseDate(IsoWeekStartDate(week));
  if (!start_date) {
    return "";
  }
  sys_days end = *start_date + days{kDaysInWeek - 1};
  return FormatDate(end);
}
