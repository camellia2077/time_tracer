// reprocessing/validator/internal/SourceFileValidator.h
#ifndef SOURCE_FILE_VALIDATOR_H
#define SOURCE_FILE_VALIDATOR_H

#include "reprocessing/validator/ValidatorUtils.h"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

class SourceFileValidator {
public:
    // 构造函数需要 remark_prefix 的配置文件路径
    SourceFileValidator(const std::string& config_filename);

    // 执行验证的核心函数
    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    std::string remark_prefix_;

    // 私有辅助函数
    void loadConfiguration(const std::string& config_filename);
    bool isDateLine(const std::string& line);
    bool isRemarkLine(const std::string& line);
    bool parseEventLine(const std::string& line, std::string& outTimeStr, std::string& outDescription);
};

#endif // SOURCE_FILE_VALIDATOR_H