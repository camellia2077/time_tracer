// converter/validator/FileValidator.cpp
#include "FileValidator.hpp"
#include "converter/validator/source_txt/facade/SourceFacade.hpp"
#include "converter/validator/output_json/facade/JsonValidator.hpp"

FileValidator::FileValidator(const ConverterConfig& config) 
    : converter_config_(config) {}

bool FileValidator::validate_source(const std::string& filename, const std::string& content, std::set<Error>& errors) {
    SourceFacade source_validator(converter_config_);
    return source_validator.validate(filename, content, errors);
}

bool FileValidator::validate_json(const std::string& filename, const nlohmann::json& content, std::set<Error>& errors, DateCheckMode mode) {
    JsonValidator json_validator(mode);
    return json_validator.validate(filename, content, errors);
}