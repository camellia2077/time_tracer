// validator/json/rules/ActivityRules.hpp
#ifndef VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_
#define VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_

#include <nlohmann/json.hpp>

#include "validator/common/ValidatorUtils.hpp"

namespace validator {
namespace json {

void validateActivityCount(const nlohmann::json& day_object,
                           std::set<Error>& errors);

}  // namespace json
}  // namespace validator

#endif  // VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_