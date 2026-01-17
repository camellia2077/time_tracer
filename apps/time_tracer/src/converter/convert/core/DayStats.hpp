// converter/convert/core/DayStats.hpp

#ifndef CONVERTER_CONVERT_CORE_DAY_STATS_HPP_
#define CONVERTER_CONVERT_CORE_DAY_STATS_HPP_

#include "common/model/DailyLog.hpp"
#include <string>

class DayStats {
public:
    void calculate_stats(DailyLog& day);

private:
    int calculateDurationSeconds(const std::string& startTimeStr, const std::string& endTimeStr) const;
    long long timeStringToTimestamp(const std::string& date, const std::string& time, bool is_end_time, long long start_timestamp_for_end) const;
};

#endif // CONVERTER_CONVERT_CORE_DAY_STATS_HPP_