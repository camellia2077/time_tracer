// domain/logic/validator/txt/facade/text_validator.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_
#define DOMAIN_LOGIC_VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_

#include <memory>
#include <set>
#include <string>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/types/converter_config.hpp"

namespace validator::txt {

class TextValidator {
 public:
  explicit TextValidator(const ConverterConfig& config);
  ~TextValidator();

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto Validate(const std::string& filename, const std::string& content,
                std::set<Error>& errors) -> bool;

 private:
  struct PImpl;
  std::unique_ptr<PImpl> pimpl_;
};

}  // namespace validator::txt

#endif  // DOMAIN_LOGIC_VALIDATOR_TXT_FACADE_TEXT_VALIDATOR_H_
