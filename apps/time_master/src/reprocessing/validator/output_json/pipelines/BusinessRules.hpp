// reprocessing/validator/output_json/pipelines/BusinessRules.hpp
#ifndef BUSINESS_RULES_HPP
#define BUSINESS_RULES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证高级业务规则
void validateHighLevelRules(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // BUSINESS_RULES_HPP