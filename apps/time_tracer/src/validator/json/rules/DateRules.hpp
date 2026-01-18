// validator/json/rules/DateRules.hpp
#ifndef VALIDATOR_JSON_RULES_DATE_RULES_HPP_
#define VALIDATOR_JSON_RULES_DATE_RULES_HPP_

#include "validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

namespace validator {
namespace json {

void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors, DateCheckMode mode);

}
}

#endif // VALIDATOR_JSON_RULES_DATE_RULES_HPP_