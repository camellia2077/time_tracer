// converter/convert/core/DayStats.hpp

#ifndef DAY_STATS_HPP
#define DAY_STATS_HPP

#include "common/model/DailyLog.hpp"
#include <string>

class DayStats {
public:
    void calculate_stats(DailyLog& day);

private:
    int calculateDurationSeconds(const std::string& startTimeStr, const std::string& endTimeStr) const;
    long long timeStringToTimestamp(const std::string& date, const std::string& time, bool is_end_time, long long start_timestamp_for_end) const;
};

#endif // DAY_STATS_HPP