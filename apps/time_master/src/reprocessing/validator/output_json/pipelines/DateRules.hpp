// reprocessing/validator/output_json/pipelines/DateRules.hpp
#ifndef DATE_RULES_HPP
#define DATE_RULES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证日期的连续性
void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors);

#endif // DATE_RULES_HPP