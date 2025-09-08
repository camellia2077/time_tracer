// reprocessing/validator/FileValidator.cpp
#include "FileValidator.hpp"

#include "reprocessing/validator/source_txt/SourceFileValidator.hpp"
#include "reprocessing/validator/output_json/facade/JsonValidator.hpp"

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
        case ValidatorType::JsonOutput: {
            JsonValidator json_validator(enable_day_count_check_for_output);
            return json_validator.validate(file_path, errors);
        }
        default:
            errors.insert({0, "Unknown validator type specified.", ErrorType::Logical});
            return false;
    }
}