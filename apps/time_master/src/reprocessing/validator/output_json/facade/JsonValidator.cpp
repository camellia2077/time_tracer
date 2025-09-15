// reprocessing/validator/output_json/facade/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include "common/AnsiColors.hpp"

// [核心修正] 引入所有独立的验证模块
#include "reprocessing/validator/output_json/pipelines/JsonValidatorDate.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorTime.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorRules.hpp"
#include "reprocessing/validator/output_json/pipelines/JsonValidatorActivities.hpp"

using json = nlohmann::json;

JsonValidator::JsonValidator(bool enable_day_count_check)
    : check_day_count_enabled_(enable_day_count_check) {}

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

    // [核心逻辑] 依次调用各个独立的验证函数
    if (check_day_count_enabled_) {
        validateDateContinuity(days_array, errors);
    }

    for (const auto& day_object : days_array) {
        validateTimeContinuity(day_object, errors);
        validateHighLevelRules(day_object, errors);
        validateActivityCount(day_object, errors);
    }

    return errors.empty();
}