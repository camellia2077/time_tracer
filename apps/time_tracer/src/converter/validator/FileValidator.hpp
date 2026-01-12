// converter/validator/FileValidator.hpp
#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "converter/validator/common/ValidatorUtils.hpp"
#include "converter/config/ConverterConfig.hpp"

#include <string>
#include <set>
#include <nlohmann/json.hpp> 

enum class ValidatorType {
    Source,
    JsonOutput 
};

class FileValidator {
public:
    // [核心修改] 构造时注入配置引用
    explicit FileValidator(const ConverterConfig& config);

    // [核心修改] 验证接口改为接收内容
    bool validate_source(const std::string& filename, const std::string& content, std::set<Error>& errors);
    
    // JSON 验证
    bool validate_json(const std::string& filename, const nlohmann::json& content, std::set<Error>& errors, DateCheckMode mode);

private:
    const ConverterConfig& converter_config_;
};

#endif // FILE_VALIDATOR_HPP