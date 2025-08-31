// reprocessing/validator/internal/JsonValidator.hpp
#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include "reprocessing/validator/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

class JsonValidator {
public:
    // [修复] 恢复构造函数，使其可以接收是否检查日期的标志
    explicit JsonValidator(bool enable_day_count_check = false);

    // 一个方法负责所有验证
    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    // [修复] 添加成员变量来存储配置
    bool check_day_count_enabled_;

    // 内部验证的辅助函数
    void validateDateContinuity(const nlohmann::json& days_array, std::set<Error>& errors);
    void validateTimeContinuity(const nlohmann::json& day_object, std::set<Error>& errors);
    void validateHighLevelRules(const nlohmann::json& day_object, std::set<Error>& errors);
};

#endif // JSON_VALIDATOR_HPP