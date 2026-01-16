// validator/output_json/facade/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <iostream>
#include "validator/output_json/rules/DateRules.hpp"
#include "validator/output_json/rules/ActivityRules.hpp"

using json = nlohmann::json;

JsonValidator::JsonValidator(DateCheckMode date_check_mode)
    : date_check_mode_(date_check_mode) {}

bool JsonValidator::validate(const std::string& filename, const json& days_array, std::set<Error>& errors) {
    errors.clear();

    if (!days_array.is_array()) {
        errors.insert({0, "JSON root is not an array in file: " + filename, ErrorType::Structural});
        return false;
    }

    validateDateContinuity(days_array, errors, date_check_mode_);

    for (const auto& day_object : days_array) {
        validateActivityCount(day_object, errors);
    }

    return errors.empty();
}