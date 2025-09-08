// reprocessing/converter/internal/converter/Converter.cpp
#include "Converter.hpp"
#include <stdexcept>

// [新增] 引入新的模块
#include "ActivityMapper.hpp"
#include "DayStatsCalculator.hpp"
#include "common/common_utils.hpp" 

// [修改] 构造函数中不再初始化 wake_keywords_
Converter::Converter(const ConverterConfig& config)
    : config_(config) {}

void Converter::transform(InputData& day) {
    day.processedActivities.clear();
    
    // [修改] 调用新的 ActivityMapper 进行转换
    ActivityMapper activity_mapper(config_);
    activity_mapper.map_activities(day);

}