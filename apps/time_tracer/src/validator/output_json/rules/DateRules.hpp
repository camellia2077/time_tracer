// validator/output_json/rules/DateRules.hpp
#ifndef DATE_RULES_HPP
#define DATE_RULES_HPP

#include "validator/common/ValidatorUtils.hpp"
#include <nlohmann/json.hpp>

void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors, DateCheckMode mode);

#endif // DATE_RULES_HPP



