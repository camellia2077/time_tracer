// infrastructure/reports/shared/utils/format/iso_week_utils.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_
#define REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_

#include <string>
#include <string_view>

#include "infrastructure/reports/shared/api/shared_api.hpp"

struct IsoWeek {
  int year = 0;
  int week = 0;
};

REPORTS_SHARED_API auto ParseIsoWeek(std::string_view input, IsoWeek& out)
    -> bool;
REPORTS_SHARED_API auto FormatIsoWeek(const IsoWeek& week) -> std::string;
REPORTS_SHARED_API auto IsoWeekFromDate(std::string_view date_str) -> IsoWeek;
REPORTS_SHARED_API auto IsoWeekStartDate(const IsoWeek& week) -> std::string;
REPORTS_SHARED_API auto IsoWeekEndDate(const IsoWeek& week) -> std::string;

#endif  // REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_
