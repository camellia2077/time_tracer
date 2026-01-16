// validator/FileValidator.hpp
#ifndef FILE_VALIDATOR_HPP
#define FILE_VALIDATOR_HPP

#include "validator/common/ValidatorUtils.hpp"
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
    explicit FileValidator(const ConverterConfig& config);

    bool validate_source(const std::string& filename, const std::string& content, std::set<Error>& errors);
    bool validate_json(const std::string& filename, const nlohmann::json& content, std::set<Error>& errors, DateCheckMode mode);

private:
    const ConverterConfig& converter_config_;
};

#endif // FILE_VALIDATOR_HPP