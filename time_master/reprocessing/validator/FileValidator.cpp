// reprocessing/validator/FileValidator.cpp
#include "FileValidator.h"
#include "reprocessing/validator/internal/SourceFileValidator.h"
#include "reprocessing/validator/internal/OutputFileValidator.h"

FileValidator::FileValidator(const std::string& source_config_path, 
                             const std::string& output_validator_config_path, 
                             const std::string& output_header_config_path)
    : source_config_path_(source_config_path),
      output_validator_config_path_(output_validator_config_path),
      output_header_config_path_(output_header_config_path) {}

bool FileValidator::validate(const std::string& file_path, 
                             ValidatorType type, 
                             std::set<Error>& errors, 
                             bool enable_day_count_check_for_output) {
    // 清空上一次的错误，确保结果的纯净性
    errors.clear();

    switch (type) {
        case ValidatorType::Source: {
            // 实例化并调用源文件验证器
            SourceFileValidator source_validator(source_config_path_);
            return source_validator.validate(file_path, errors);
        }
        case ValidatorType::Output: {
            // 实例化并调用转换后文件验证器
            OutputFileValidator output_validator(
                output_validator_config_path_,
                output_header_config_path_,
                enable_day_count_check_for_output
            );
            return output_validator.validate(file_path, errors);
        }
        default:
            // 处理未知的验证类型，虽然在当前设计下不太可能发生
            errors.insert({0, "Unknown validator type specified.", ErrorType::Logical});
            return false;
    }
}