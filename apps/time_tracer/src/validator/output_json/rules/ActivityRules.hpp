// validator/output_json/rules/ActivityRules.hpp
#ifndef ACTIVITY_RULES_HPP
#define ACTIVITY_RULES_HPP

#include "validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

void validateActivityCount(const nlohmann::json& day_object, std::set<Error>& errors);

#endif // ACTIVITY_RULES_HPP