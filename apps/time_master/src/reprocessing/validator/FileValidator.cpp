// reprocessing/validator/FileValidator.cpp
#include "FileValidator.hpp"

#include "reprocessing/validator/source_txt/facade/SourceFacade.hpp"
#include "reprocessing/validator/output_json/facade/JsonValidator.hpp"

// [修改] 构造函数现在加载并存储 ConverterConfig
FileValidator::FileValidator(const std::string& main_config_path) {
    converter_config_ = std::make_unique<ConverterConfig>();
    // 如果加载失败，converter_config_ 将处于未完全初始化的状态，
    // 后续的验证会因为缺少关键字而自然失败。
    converter_config_->load(main_config_path);
}

bool FileValidator::validate(const std::string& file_path, 
                             ValidatorType type, 
                             std::set<Error>& errors, 
                             bool enable_day_count_check_for_output) {
    errors.clear();

    switch (type) {
        case ValidatorType::Source: {
            // [修改] 使用存储的 converter_config_ 来初始化 SourceFacade
            SourceFacade source_validator(*converter_config_);
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