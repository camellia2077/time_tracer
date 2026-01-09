// reprocessing/validator/FileValidator.cpp
#include "FileValidator.hpp"
#include "reprocessing/validator/source_txt/facade/SourceFacade.hpp"
#include "reprocessing/validator/output_json/facade/JsonValidator.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp" 


FileValidator::FileValidator(const std::filesystem::path& main_config_path) 
    : converter_config_(std::make_unique<ConverterConfig>()) {
    converter_config_->load(main_config_path); 
}


bool FileValidator::validate(const std::filesystem::path& file_path, 
                             ValidatorType type, 
                             std::set<Error>& errors, 
                             DateCheckMode date_check_mode_for_output) {
    errors.clear();

    switch (type) {
        case ValidatorType::Source: {
            if (converter_config_) {
                SourceFacade source_validator(*converter_config_);
                return source_validator.validate(file_path, errors);
            } 
            else {
                 errors.insert({0, "Internal Error: Converter configuration not initialized.", ErrorType::Logical});
                 return false;
            }
        }
        case ValidatorType::JsonOutput: {
            // 传递枚举值
            JsonValidator json_validator(date_check_mode_for_output);
            return json_validator.validate(file_path, errors);
        }
        default:
            errors.insert({0, "Unknown validator type specified.", ErrorType::Logical});
            return false;
    }
}
