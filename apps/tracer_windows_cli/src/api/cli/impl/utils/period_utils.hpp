// api/cli/impl/utils/period_utils.hpp
#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

#include "domain/utils/time_utils.hpp"
#include "shared/utils/period_utils.hpp"

namespace time_tracer::cli::impl::utils {

class PeriodParser {
public:
  struct PeriodArgs {
    std::string_view period_type_;
    std::string_view period_arg_;
  };

  static auto Normalize(PeriodArgs args) -> std::string {
    if (args.period_type_ == "day") {
      return NormalizeToDateFormat(std::string(args.period_arg_));
    }
    if (args.period_type_ == "month") {
      return NormalizeToMonthFormat(std::string(args.period_arg_));
    }
    if (args.period_type_ == "week") {
      IsoWeek week{};
      if (!ParseIsoWeek(std::string(args.period_arg_), week)) {
        throw std::runtime_error(
            "Invalid ISO week format. Use YYYY-Www (e.g., 2026-W05).");
      }
      return FormatIsoWeek(week);
    }
    if (args.period_type_ == "year") {
      int gregorian_year = 0;
      if (!ParseGregorianYear(std::string(args.period_arg_), gregorian_year)) {
        throw std::runtime_error(
            "Invalid year format. Use Gregorian YYYY (e.g., 2026).");
      }
      return FormatGregorianYear(gregorian_year);
    }
    if (args.period_type_ == "recent" || args.period_type_ == "all-recent") {
      return std::string(
          args.period_arg_); // Handled by ParseNumberList elsewhere if needed
    }
    return std::string(args.period_arg_);
  }
};

} // namespace time_tracer::cli::impl::utils
