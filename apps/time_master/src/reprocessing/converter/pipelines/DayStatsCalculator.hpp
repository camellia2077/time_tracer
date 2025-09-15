// reprocessing/converter/pipelines/DayStatsCalculator.hpp

#ifndef DAY_STATS_CALCULATOR_HPP
#define DAY_STATS_CALCULATOR_HPP

#include "reprocessing/converter/model/InputData.hpp"
#include <string>

class DayStatsCalculator {
public:
    void calculate_stats(InputData& day);

private:
    int calculateDurationSeconds(const std::string& startTimeStr, const std::string& endTimeStr) const;
    long long timeStringToTimestamp(const std::string& date, const std::string& time, bool is_end_time, long long start_timestamp_for_end) const;
};

#endif // DAY_STATS_CALCULATOR_HPP