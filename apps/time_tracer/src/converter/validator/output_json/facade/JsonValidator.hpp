// converter/validator/output_json/facade/JsonValidator.hpp
#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include "converter/validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

class JsonValidator {
public:
    explicit JsonValidator(DateCheckMode date_check_mode = DateCheckMode::None);

    // [核心修改] 接收 JSON 对象
    bool validate(const std::string& filename, const nlohmann::json& json_content, std::set<Error>& errors);

private:
    DateCheckMode date_check_mode_;
};

#endif // JSON_VALIDATOR_HPP