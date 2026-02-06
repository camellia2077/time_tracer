// validator/json/facade/json_validator.cpp
#include "validator/json/facade/json_validator.hpp"

#include <iostream>

#include "validator/json/rules/activity_rules.hpp"
#include "validator/json/rules/date_rules.hpp"

namespace validator::json {

JsonValidator::JsonValidator(DateCheckMode date_check_mode)
    : date_check_mode_(date_check_mode) {}

// [修改] 参数类型 json -> nlohmann::json
auto JsonValidator::Validate(const std::string& filename,
                             const nlohmann::json& json_content,
                             std::set<Error>& errors) -> bool {

  errors.clear();

  if (!json_content.is_array()) {
    errors.insert({0, "JSON root is not an array in file: " + filename,
                   ErrorType::kStructural});
    return false;
  }

  validateDateContinuity(json_content, errors, date_check_mode_);

  for (const auto& day_object : json_content) {
    validateActivityCount(day_object, errors);
  }

  return errors.empty();
}

}  // namespace validator::json
