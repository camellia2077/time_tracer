// domain/logic/validator/txt/rules/line_rules.hpp
#ifndef VALIDATOR_TXT_RULES_LINE_RULES_H_
#define VALIDATOR_TXT_RULES_LINE_RULES_H_

// [Fix] 修改头文件路径：指向重构后的位置
#include <optional>
#include <set>
#include <string>
#include <unordered_set>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/model/source_span.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

namespace validator::txt {

class LineRules {
 public:
  explicit LineRules(const ConverterConfig& config);

  static auto IsYear(const std::string& line) -> bool;
  static auto IsDate(const std::string& line) -> bool;
  auto IsRemark(const std::string& line) const -> bool;

  auto IsValidEventLine(const std::string& line, int line_number,
                        std::set<Error>& errors,
                        const std::optional<SourceSpan>& span) const -> bool;

 private:
  const ConverterConfig& config_;
  std::unordered_set<std::string> valid_event_keywords_;
  std::unordered_set<std::string> wake_keywords_;
};

}  // namespace validator::txt

#endif  // VALIDATOR_TXT_RULES_LINE_RULES_H_
