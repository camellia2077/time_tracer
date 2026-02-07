// domain/logic/converter/convert/core/day_stats.hpp
#ifndef CONVERTER_CONVERT_CORE_DAY_STATS_H_
#define CONVERTER_CONVERT_CORE_DAY_STATS_H_

#include <string>

#include "domain/model/daily_log.hpp"

class DayStats {
 public:
  static void CalculateStats(DailyLog& day);

 private:
  static auto CalculateDurationSeconds(const std::string& start_time_str,
                                       const std::string& end_time_str) -> int;
  static auto TimeStringToTimestamp(const std::string& date,
                                    const std::string& time, bool is_end_time,
                                    long long start_timestamp_for_end)
      -> long long;
};

#endif  // CONVERTER_CONVERT_CORE_DAY_STATS_H_
