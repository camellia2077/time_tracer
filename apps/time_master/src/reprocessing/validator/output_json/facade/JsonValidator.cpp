// reprocessing/validator/output_json/JsonValidator.cpp
#include "JsonValidator.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <vector>

using json = nlohmann::json;

JsonValidator::JsonValidator(bool enable_day_count_check)
    : check_day_count_enabled_(enable_day_count_check) {}

bool JsonValidator::validate(const std::string& file_path, std::set<Error>& errors) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        errors.insert({0, "Could not open JSON file: " + file_path, ErrorType::FileAccess});
        return false;
    }

    json days_array;
    try {
        file >> days_array;
        if (!days_array.is_array()) {
            errors.insert({0, "JSON root is not an array. The file should contain an array of day objects.", ErrorType::Structural});
            return false;
        }
    } catch (const json::parse_error& e) {
        errors.insert({0, "Failed to parse JSON file: " + std::string(e.what()), ErrorType::Structural});
        return false;
    }

    if (days_array.empty()) {
        return true;
    }

    // 调用独立的验证函数
    if (check_day_count_enabled_) {
        validateDateContinuity(days_array, errors);
    }

    for (const auto& day_object : days_array) {
        if (!day_object.is_object()) continue;
        
        validateTimeContinuity(day_object, errors);
        validateHighLevelRules(day_object, errors);
        validateActivityCount(day_object, errors);
    }
    
    return errors.empty();
}