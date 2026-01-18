// converter/convert/core/activity_mapper.hpp
#ifndef CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_
#define CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_

#include "common/model/daily_log.hpp"

#include "converter/config/converter_config.hpp"

#include <string>
#include <unordered_set>

class ActivityMapper {
public:
    explicit ActivityMapper(const ConverterConfig& config);
    void map_activities(DailyLog& day);

private:
    const ConverterConfig& config_;
    const std::unordered_set<std::string> wake_keywords_;

    // 私有辅助函数
    std::string formatTime(const std::string& timeStrHHMM) const;
    int calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) const;
};

#endif // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_