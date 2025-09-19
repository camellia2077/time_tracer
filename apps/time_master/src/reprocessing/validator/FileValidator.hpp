// reprocessing/validator/FileValidator.hpp
#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include "reprocessing/converter/config/ConverterConfig.hpp" // [新增]
#include <string>
#include <set>
#include <memory> // [新增]

enum class ValidatorType {
    Source,
    JsonOutput 
};

class FileValidator {
public:
    // [修改] 构造函数仍然接收主配置文件路径
    FileValidator(const std::string& main_config_path);

    bool validate(const std::string& file_path, 
                  ValidatorType type, 
                  std::set<Error>& errors, 
                  bool enable_day_count_check_for_output = false);

private:
    // [修改] 存储加载好的 ConverterConfig
    std::unique_ptr<ConverterConfig> converter_config_;
};

#endif // FILE_VALIDATOR_HPP