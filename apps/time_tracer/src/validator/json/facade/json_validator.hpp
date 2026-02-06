// validator/json/facade/json_validator.hpp
#ifndef VALIDATOR_JSON_FACADE_JSON_VALIDATOR_H_
#define VALIDATOR_JSON_FACADE_JSON_VALIDATOR_H_

#include <nlohmann/json.hpp>
#include <set>
#include <string>

#include "validator/common/validator_utils.hpp"

namespace validator::json {

class JsonValidator {
 public:
  explicit JsonValidator(DateCheckMode date_check_mode = DateCheckMode::kNone);
  auto Validate(const std::string& filename, const nlohmann::json& json_content,
                std::set<Error>& errors) -> bool;

 private:
  DateCheckMode date_check_mode_;
};

}  // namespace validator::json

#endif  // VALIDATOR_JSON_FACADE_JSON_VALIDATOR_H_