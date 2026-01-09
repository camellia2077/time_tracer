// reprocessing/validator/output_json/facade/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include "common/AnsiColors.hpp"

#include "reprocessing/validator/output_json/pipelines/DateRules.hpp"
#include "reprocessing/validator/output_json/pipelines/ActivityRules.hpp"

using json = nlohmann::json;

JsonValidator::JsonValidator(DateCheckMode date_check_mode)
    : date_check_mode_(date_check_mode) {}

bool JsonValidator::validate(const std::filesystem::path& file_path, std::set<Error>& errors) {
    errors.clear();

    std::ifstream file(file_path);
    // [修正] 变量名修正为 file，且路径转为 string
    if (!file.is_open()) {
        errors.insert({0, "Could not open file: " + file_path.string(), ErrorType::FileAccess});
        return false;
    }

    json days_array;
    try {
        file >> days_array;
        // [修正] 路径转为 string
        if (!days_array.is_array()) {
            errors.insert({0, "JSON root is not an array in file: " + file_path.string(), ErrorType::Structural});
            return false;
        }
    } catch (const json::parse_error& e) {
        errors.insert({0, "Failed to parse JSON: " + std::string(e.what()), ErrorType::Structural});
        return false;
    }

    validateDateContinuity(days_array, errors, date_check_mode_);

    for (const auto& day_object : days_array) {
        validateActivityCount(day_object, errors);
    }

    return errors.empty();
}