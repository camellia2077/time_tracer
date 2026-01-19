// converter/convert/core/activity_mapper.hpp
#ifndef CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_
#define CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_

#include "common/model/daily_log.hpp"
#include "common/config/models/converter_config_models.hpp"

#include <string>
#include <vector>

class ActivityMapper {
public:
    explicit ActivityMapper(const ConverterConfig& config);
    void map_activities(DailyLog& day);

private:
    const ConverterConfig& config_;
    // 保持引用以避免拷贝，或者拷贝 vector (取决于数据量，引用通常更高效)
    const std::vector<std::string>& wake_keywords_;

    std::string formatTime(const std::string& timeStrHHMM) const;
    int calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) const;
};

#endif // CONVERTER_CONVERT_CORE_ACTIVITY_MAPPER_HPP_