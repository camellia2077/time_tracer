// domain/logic/validator/json/rules/activity_rules.hpp
#ifndef VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_
#define VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_

#include <nlohmann/json.hpp>

#include "domain/logic/validator/common/validator_utils.hpp"

namespace validator::json {

void ValidateActivityCount(const nlohmann::json& day_object,
                           std::set<Error>& errors);

}  // namespace validator::json

#endif  // VALIDATOR_JSON_RULES_ACTIVITY_RULES_H_
