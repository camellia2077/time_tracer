// domain/logic/validator/json/rules/json_rules.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_JSON_RULES_JSON_RULES_H_
#define DOMAIN_LOGIC_VALIDATOR_JSON_RULES_JSON_RULES_H_

#include <nlohmann/json.hpp>

#include "domain/logic/validator/common/validator_utils.hpp"

namespace validator::json {

void ValidateActivityCount(const nlohmann::json& day_object,
                           std::set<Error>& errors);

void ValidateDateContinuity(const nlohmann::json& days_array,
                            std::set<Error>& errors, DateCheckMode mode);

}  // namespace validator::json

#endif  // DOMAIN_LOGIC_VALIDATOR_JSON_RULES_JSON_RULES_H_
