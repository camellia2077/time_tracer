// reprocessing/validator/output_json/pipelines/JsonValidatorRules.hpp
#ifndef JSON_VALIDATOR_RULES_HPP
#define JSON_VALIDATOR_RULES_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

// 独立函数，负责验证高级业务规则
void validateHighLevelRules(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // JSON_VALIDATOR_RULES_HPP