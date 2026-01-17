// validator/json/facade/JsonValidator.hpp
#ifndef VALIDATOR_JSON_FACADE_JSON_VALIDATOR_HPP_
#define VALIDATOR_JSON_FACADE_JSON_VALIDATOR_HPP_

#include "validator/common/ValidatorUtils.hpp"
#include <string>
#include <set>
#include <nlohmann/json.hpp>

namespace validator {
namespace json {

class JsonValidator {
public:
    explicit JsonValidator(DateCheckMode date_check_mode = DateCheckMode::None);
    bool validate(const std::string& filename, const nlohmann::json& json_content, std::set<Error>& errors);

private:
    DateCheckMode date_check_mode_;
};

} // namespace json
} // namespace validator

#endif // VALIDATOR_JSON_FACADE_JSON_VALIDATOR_HPP_