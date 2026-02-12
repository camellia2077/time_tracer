// domain/logic/validator/json/rules/date_rules.hpp
#ifndef VALIDATOR_JSON_RULES_DATE_RULES_H_
#define VALIDATOR_JSON_RULES_DATE_RULES_H_

#include <nlohmann/json.hpp>

#include "domain/logic/validator/common/validator_utils.hpp"

namespace validator::json {

void ValidateDateContinuity(const nlohmann::json& days_array,
                            std::set<Error>& errors, DateCheckMode mode);

}  // namespace validator::json

#endif  // VALIDATOR_JSON_RULES_DATE_RULES_H_
