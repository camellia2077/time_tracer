// reprocessing/converter/pipelines/ActivityMapper.hpp
#ifndef ACTIVITY_MAPPER_HPP
#define ACTIVITY_MAPPER_HPP

#include "reprocessing/converter/model/InputData.hpp"

#include "reprocessing/converter/config/ConverterConfig.hpp"
#include <string>
#include <unordered_set>

class ActivityMapper {
public:
    explicit ActivityMapper(const ConverterConfig& config);
    void map_activities(InputData& day);

private:
    const ConverterConfig& config_;
    const std::unordered_set<std::string> wake_keywords_;

    // 私有辅助函数
    std::string formatTime(const std::string& timeStrHHMM) const;
    int calculateDurationMinutes(const std::string& startTimeStr, const std::string& endTimeStr) const;
};

#endif // ACTIVITY_MAPPER_HPP