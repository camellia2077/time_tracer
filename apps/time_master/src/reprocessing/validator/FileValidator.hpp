// reprocessing/validator/FileValidator.hpp
#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp"
#include <string>
#include <set>
#include <memory>

enum class ValidatorType {
    Source,
    JsonOutput 
};

class FileValidator {
public:
    FileValidator(const std::string& main_config_path);

    // [核心修改] 最后一个参数类型变更为 DateCheckMode
    bool validate(const std::string& file_path, 
                  ValidatorType type, 
                  std::set<Error>& errors, 
                  DateCheckMode date_check_mode_for_output = DateCheckMode::None);

private:
    std::unique_ptr<ConverterConfig> converter_config_;
};

#endif // FILE_VALIDATOR_HPP