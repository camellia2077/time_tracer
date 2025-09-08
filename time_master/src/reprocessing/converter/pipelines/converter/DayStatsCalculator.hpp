// reprocessing/converter/internal/DayStatsCalculator.hpp


#ifndef DAY_STATS_CALCULATOR_HPP
#define DAY_STATS_CALCULATOR_HPP
// 这个模块将负责基于 ActivityMapper 转换后的数据进行统计计算。

#include "reprocessing/converter/model/InputData.hpp"
#include <string>

class DayStatsCalculator {
public:
    void calculate_stats(InputData& day);
};

#endif // DAY_STATS_CALCULATOR_HPP