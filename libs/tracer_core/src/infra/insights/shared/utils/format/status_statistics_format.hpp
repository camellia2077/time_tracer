// infra/insights/shared/utils/format/status_statistics_format.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_STATUS_STATISTICS_FORMAT_H_
#define INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_STATUS_STATISTICS_FORMAT_H_

#include <cstdint>
#include <string>

#include "infra/insights/shared/utils/format/time_format.hpp"

inline auto FormatStatusStatistics(int occurrence_count,
                                   std::int64_t total_duration,
                                   const std::string& count_unit) -> std::string {
  return std::to_string(occurrence_count) + " " + count_unit + " (" +
         TimeFormatDuration(total_duration) + ")";
}

#endif  // INFRASTRUCTURE_INSIGHTS_SHARED_UTILS_FORMAT_STATUS_STATISTICS_FORMAT_H_
