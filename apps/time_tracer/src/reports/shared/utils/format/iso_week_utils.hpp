// reports/shared/utils/format/iso_week_utils.hpp
#ifndef REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_
#define REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_

#include <string>
#include <string_view>

#include "reports/shared/api/shared_api.hpp"

struct IsoWeek {
  int year = 0;
  int week = 0;
};

REPORTS_SHARED_API bool parse_iso_week(std::string_view input, IsoWeek& out);
REPORTS_SHARED_API std::string format_iso_week(const IsoWeek& week);
REPORTS_SHARED_API IsoWeek iso_week_from_date(std::string_view date_str);
REPORTS_SHARED_API std::string iso_week_start_date(const IsoWeek& week);
REPORTS_SHARED_API std::string iso_week_end_date(const IsoWeek& week);

#endif  // REPORTS_SHARED_UTILS_FORMAT_ISO_WEEK_UTILS_H_
