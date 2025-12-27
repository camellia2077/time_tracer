// reprocessing/validator/output_json/pipelines/ActivityRules.hpp
#ifndef ACTIVITY_RULES_HPP
#define ACTIVITY_RULES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证单日内的活动数量
void validateActivityCount(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // ACTIVITY_RULES_HPP