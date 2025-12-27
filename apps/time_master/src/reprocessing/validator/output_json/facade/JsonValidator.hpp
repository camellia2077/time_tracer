// reprocessing/validator/output_json/facade/JsonValidator.hpp
#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include "reprocessing/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

// ... Includes ...

class JsonValidator {
public:
    /**
     * @brief 构造函数。
     * @param date_check_mode (可选) 日期完整性检查模式。
     * 默认为 DateCheckMode::None。
     */
    // [核心修改] 参数类型变更
    explicit JsonValidator(DateCheckMode date_check_mode = DateCheckMode::None);

    bool validate(const std::string& file_path, std::set<Error>& errors);

private:
    /// @brief 存储检查模式
    DateCheckMode date_check_mode_;
};

#endif // JSON_VALIDATOR_HPP