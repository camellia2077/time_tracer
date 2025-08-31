// reprocessing/validator/FileValidator.cpp
#include "FileValidator.hpp"
#include "reprocessing/validator/internal/SourceFileValidator.hpp"
#include "reprocessing/validator/internal/JsonValidator.hpp" // [修改] 引入新的JsonValidator

FileValidator::FileValidator(const std::string& source_config_path)
    : source_config_path_(source_config_path) {}

bool FileValidator::validate(const std::string& file_path, 
                             ValidatorType type, 
                             std::set<Error>& errors, 
                             bool enable_day_count_check_for_output) {
    errors.clear();

    switch (type) {
        case ValidatorType::Source: {
            SourceFileValidator source_validator(source_config_path_);
            return source_validator.validate(file_path, errors);
        }
        case ValidatorType::JsonOutput: { // [修改]
            JsonValidator json_validator(enable_day_count_check_for_output);
            return json_validator.validate(file_path, errors);
        }
        default:
            errors.insert({0, "Unknown validator type specified.", ErrorType::Logical});
            return false;
    }
}