// api/cli/impl/utils/tree_formatter.cpp
#include "api/cli/impl/utils/tree_formatter.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace {

auto PrintNodeRecursive(const ProjectTreeNode &node, const std::string &prefix,
                        bool is_last, int current_depth) -> void {
  std::cout << prefix;
  if (current_depth > 0) {
    std::cout << (is_last ? "└── " : "├── ");
  }

  std::cout << node.name << "\n";

  std::string new_prefix = prefix;
  if (current_depth > 0) {
    new_prefix += (is_last ? "    " : "│   ");
  }

  for (size_t i = 0; i < node.children.size(); ++i) {
    PrintNodeRecursive(node.children[i], new_prefix,
                       i == node.children.size() - 1, current_depth + 1);
  }
}

} // namespace

auto TreeFormatter::PrintRoots(const std::vector<std::string> &roots) -> void {
  std::cout << "Root Projects:\n";
  for (const auto &root : roots) {
    std::cout << "- " << root << "\n";
  }
}

auto TreeFormatter::PrintTree(const std::vector<ProjectTreeNode> &nodes)
    -> void {
  for (const auto &node : nodes) {
    PrintNodeRecursive(node, "", true, 0);
  }
}

namespace tracer_core::cli::impl::utils {

namespace {
constexpr int kLeapYearDivisor400 = 400;
constexpr int kLeapYearDivisor100 = 100;
constexpr int kLeapYearDivisor4 = 4;

constexpr int kDaysInLongMonth = 31;
constexpr int kDaysInShortMonth = 30;
constexpr int kDaysInLeapFebruary = 29;
constexpr int kDaysInStandardFebruary = 28;

constexpr int kFebruary = 2;
constexpr int kApril = 4;
constexpr int kJune = 6;
constexpr int kSeptember = 9;
constexpr int kNovember = 11;

constexpr int kThresholdTwoDigits = 10;
constexpr int kIsoYearLength = 4;
constexpr int kIsoMonthLength = 6;
constexpr int kIsoDateLength = 8;

constexpr int kYearOffset = 0;
constexpr int kMonthOffset = 4;
constexpr int kDayOffset = 6;

constexpr int kMonthLength = 2;
constexpr int kDayLength = 2;

constexpr int kJanuary = 1;
constexpr int kDecember = 12;
constexpr int kFirstDayOfMonth = 1;
constexpr int kLastDayOfYear = 31;
} // namespace

auto IsLeapYear(int year) -> bool {
  if (year % kLeapYearDivisor400 == 0) {
    return true;
  }
  if (year % kLeapYearDivisor100 == 0) {
    return false;
  }
  return (year % kLeapYearDivisor4 == 0);
}

auto DaysInMonth(const MonthInfo &info) -> int {
  switch (info.month) {
  case kFebruary:
    return IsLeapYear(info.year) ? kDaysInLeapFebruary
                                 : kDaysInStandardFebruary;
  case kApril:
  case kJune:
  case kSeptember:
  case kNovember:
    return kDaysInShortMonth;
  default:
    return kDaysInLongMonth;
  }
}

auto FormatDate(const DateParts &parts) -> std::string {
  std::string month_str = (parts.month < kThresholdTwoDigits)
                              ? ("0" + std::to_string(parts.month))
                              : std::to_string(parts.month);
  std::string day_str = (parts.day < kThresholdTwoDigits)
                            ? ("0" + std::to_string(parts.day))
                            : std::to_string(parts.day);
  return std::to_string(parts.year) + "-" + month_str + "-" + day_str;
}

auto NormalizeDateInput(const std::string &input, bool is_end) -> std::string {
  std::string digits;
  digits.reserve(input.size());
  for (char character : input) {
    if (std::isdigit(static_cast<unsigned char>(character)) != 0) {
      digits.push_back(character);
    }
  }

  if (digits.size() == kIsoYearLength) {
    int year = std::stoi(digits);
    if (is_end) {
      return FormatDate(
          {.year = year, .month = kDecember, .day = kLastDayOfYear});
    }
    return FormatDate(
        {.year = year, .month = kJanuary, .day = kFirstDayOfMonth});
  }
  if (digits.size() == kIsoMonthLength) {
    int year = std::stoi(digits.substr(kYearOffset, kIsoYearLength));
    int month = std::stoi(digits.substr(kMonthOffset, kMonthLength));
    if (is_end) {
      return FormatDate({.year = year,
                         .month = month,
                         .day = DaysInMonth({.year = year, .month = month})});
    }
    return FormatDate({.year = year, .month = month, .day = kFirstDayOfMonth});
  }
  if (digits.size() == kIsoDateLength) {
    int year = std::stoi(digits.substr(kYearOffset, kIsoYearLength));
    int month = std::stoi(digits.substr(kMonthOffset, kMonthLength));
    int day = std::stoi(digits.substr(kDayOffset, kDayLength));
    return FormatDate({.year = year, .month = month, .day = day});
  }

  throw std::runtime_error(
      "Invalid date input. Use YYYY, YYYYMM, or YYYYMMDD.");
}

} // namespace tracer_core::cli::impl::utils
