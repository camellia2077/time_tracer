// shared/utils/period_utils.hpp
#ifndef SHARED_UTILS_PERIOD_UTILS_H_
#define SHARED_UTILS_PERIOD_UTILS_H_

#include <string>
#include <string_view>

struct IsoWeek {
  int year = 0;
  int week = 0;
};

auto ParseGregorianYear(std::string_view input, int& gregorian_year) -> bool;
auto FormatGregorianYear(int gregorian_year) -> std::string;

auto ParseIsoWeek(std::string_view input, IsoWeek& out) -> bool;
auto FormatIsoWeek(const IsoWeek& week) -> std::string;
auto IsoWeekFromDate(std::string_view date_str) -> IsoWeek;
auto IsoWeekStartDate(const IsoWeek& week) -> std::string;
auto IsoWeekEndDate(const IsoWeek& week) -> std::string;

#endif  // SHARED_UTILS_PERIOD_UTILS_H_
