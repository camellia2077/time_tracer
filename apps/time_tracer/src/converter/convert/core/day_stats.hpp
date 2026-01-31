// converter/convert/core/day_stats.hpp
#ifndef CONVERTER_CONVERT_CORE_DAY_STATS_H_
#define CONVERTER_CONVERT_CORE_DAY_STATS_H_

#include <string>

#include "domain/model/daily_log.hpp"

class DayStats {
 public:
  void calculate_stats(DailyLog& day);

 private:
  static int calculateDurationSeconds(const std::string& startTimeStr,
                                      const std::string& endTimeStr);
  static long long timeStringToTimestamp(const std::string& date,
                                         const std::string& time,
                                         bool is_end_time,
                                         long long start_timestamp_for_end);
};

#endif  // CONVERTER_CONVERT_CORE_DAY_STATS_H_
