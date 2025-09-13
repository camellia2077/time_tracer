// reprocessing/validator/output_json/pipelines/JsonValidatorTime.hpp
#ifndef JSON_VALIDATOR_TIME_HPP
#define JSON_VALIDATOR_TIME_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证一天内活动的时间是否连续
void validateTimeContinuity(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // JSON_VALIDATOR_TIME_HPP