// validator/json/rules/ActivityRules.hpp
#ifndef VALIDATOR_JSON_RULES_ACTIVITY_RULES_HPP_
#define VALIDATOR_JSON_RULES_ACTIVITY_RULES_HPP_

#include "validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

namespace validator {
namespace json {

void validateActivityCount(const nlohmann::json& day_object, std::set<Error>& errors);

} // namespace json
} // namespace validator

#endif // VALIDATOR_JSON_RULES_ACTIVITY_RULES_HPP_