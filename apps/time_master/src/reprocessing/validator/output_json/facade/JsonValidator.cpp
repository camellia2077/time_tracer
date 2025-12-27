// reprocessing/validator/output_json/facade/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include "common/AnsiColors.hpp"

// [核心修正] 引入所有独立的验证模块
#include "reprocessing/validator/output_json/pipelines/DateRules.hpp"
#include "reprocessing/validator/output_json/pipelines/TimeRules.hpp"
#include "reprocessing/validator/output_json/pipelines/BusinessRules.hpp"
#include "reprocessing/validator/output_json/pipelines/ActivityRules.hpp"

using json = nlohmann::json;

JsonValidator::JsonValidator(DateCheckMode date_check_mode)
    : date_check_mode_(date_check_mode) {}

bool JsonValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    errors.clear();

    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    json days_array;
    try {
        file >> days_array;
        if (!days_array.is_array()) {
            errors.insert({0, "JSON root is not an array in file: " + file_path, ErrorType::Structural});
            return false;
        }
    } catch (const json::parse_error& e) {
        errors.insert({0, "Failed to parse JSON: " + std::string(e.what()), ErrorType::Structural});
        return false;
    }


    validateDateContinuity(days_array, errors, date_check_mode_);


    for (const auto& day_object : days_array) {
        validateTimeContinuity(day_object, errors);
        validateHighLevelRules(day_object, errors);
        validateActivityCount(day_object, errors);
    }

    return errors.empty();
}