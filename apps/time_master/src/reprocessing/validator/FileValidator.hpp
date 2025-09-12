// reprocessing/validator/FileValidator.hpp
#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>

// [修改] 更新枚举以反映新的验证目标
enum class ValidatorType {
    Source,
    JsonOutput 
};

class FileValidator {
public:
    // [修改] 构造函数参数简化，不再需要 output_header_config
    FileValidator(const std::string& source_config_path);

    bool validate(const std::string& file_path, 
                  ValidatorType type, 
                  std::set<Error>& errors, 
                  bool enable_day_count_check_for_output = false);

private:
    std::string source_config_path_;
};

#endif // FILE_VALIDATOR_HPP