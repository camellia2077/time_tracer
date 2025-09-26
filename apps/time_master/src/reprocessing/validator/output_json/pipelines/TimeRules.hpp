// reprocessing/validator/output_json/pipelines/TimeRules.hpp
#ifndef TIME_RULES_HPP
#define TIME_RULES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证一天内活动的时间是否连续
void validateTimeContinuity(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // TIME_RULES_HPP