// validator/txt/facade/TextValidator.hpp
#ifndef VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_
#define VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_

#include "validator/common/ValidatorUtils.hpp"

// [Fix] 修改头文件路径：指向重构后的位置
#include <memory>
#include <set>
#include <string>

#include "common/config/models/converter_config_models.hpp"

namespace validator {
namespace txt {

class TextValidator {
 public:
  explicit TextValidator(const ConverterConfig& config);
  ~TextValidator();

  bool validate(const std::string& filename, const std::string& content,
                std::set<Error>& errors);

 private:
  struct PImpl;
  std::unique_ptr<PImpl> pimpl_;
};

}  // namespace txt
}  // namespace validator

#endif  // VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_