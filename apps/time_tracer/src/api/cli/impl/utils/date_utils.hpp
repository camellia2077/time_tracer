// api/cli/impl/utils/date_utils.hpp
#pragma once

#include <string>

namespace time_tracer::cli::impl::utils {

struct DateParts {
  int year;
  int month;
  int day;
};

struct MonthInfo {
  int year;
  int month;
};

auto IsLeapYear(int year) -> bool;

auto DaysInMonth(const MonthInfo& info) -> int;

auto FormatDate(const DateParts& parts) -> std::string;

auto NormalizeDateInput(const std::string& input, bool is_end) -> std::string;

}  // namespace time_tracer::cli::impl::utils
