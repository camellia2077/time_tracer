// validator/json/facade/JsonValidator.cpp
#include "JsonValidator.hpp"

#include <iostream>

#include "validator/json/rules/ActivityRules.hpp"
#include "validator/json/rules/DateRules.hpp"

namespace validator::json {

JsonValidator::JsonValidator(DateCheckMode date_check_mode)
    : date_check_mode_(date_check_mode) {}

// [修改] 参数类型 json -> nlohmann::json
auto JsonValidator::validate(const std::string& filename,
                             const nlohmann::json& days_array,
                             std::set<Error>& errors) -> bool {
  errors.clear();

  if (!days_array.is_array()) {
    errors.insert({0, "JSON root is not an array in file: " + filename,
                   ErrorType::Structural});
    return false;
  }

  validateDateContinuity(days_array, errors, date_check_mode_);

  for (const auto& day_object : days_array) {
    validateActivityCount(day_object, errors);
  }

  return errors.empty();
}

}  // namespace validator::json
