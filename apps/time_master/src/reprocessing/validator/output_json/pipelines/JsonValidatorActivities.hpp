// reprocessing/validator/output_json/pipelines/JsonValidatorActivities.hpp
#ifndef JSON_VALIDATOR_ACTIVITIES_HPP
#define JSON_VALIDATOR_ACTIVITIES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证单日内的活动数量
void validateActivityCount(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // JSON_VALIDATOR_ACTIVITIES_HPP