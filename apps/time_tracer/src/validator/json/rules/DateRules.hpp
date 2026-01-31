// validator/json/rules/DateRules.hpp
#ifndef VALIDATOR_JSON_RULES_DATE_RULES_H_
#define VALIDATOR_JSON_RULES_DATE_RULES_H_

#include <nlohmann/json.hpp>

#include "validator/common/ValidatorUtils.hpp"

namespace validator {
namespace json {

void validateDateContinuity(const nlohmann::json& days_array,
                            std::set<Error>& errors, DateCheckMode mode);

}
}  // namespace validator

#endif  // VALIDATOR_JSON_RULES_DATE_RULES_H_