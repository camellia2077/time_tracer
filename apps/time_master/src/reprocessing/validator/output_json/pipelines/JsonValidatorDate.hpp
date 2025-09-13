// reprocessing/validator/output_json/pipelines/JsonValidatorDate.hpp
#ifndef JSON_VALIDATOR_DATE_HPP
#define JSON_VALIDATOR_DATE_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证日期的连续性
void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors);

#endif // JSON_VALIDATOR_DATE_HPP