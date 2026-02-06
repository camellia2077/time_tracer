// validator/txt/rules/line_rules.hpp
#ifndef VALIDATOR_TXT_RULES_LINE_RULES_H_
#define VALIDATOR_TXT_RULES_LINE_RULES_H_

// [Fix] 修改头文件路径：指向重构后的位置
#include <set>
#include <string>
#include <unordered_set>

#include "common/config/models/converter_config_models.hpp"
#include "validator/common/validator_utils.hpp"

namespace validator {
namespace txt {

class LineRules {
 public:
  explicit LineRules(const ConverterConfig& config);

  static bool is_year(const std::string& line);
  static bool is_date(const std::string& line);
  bool is_remark(const std::string& line) const;

  bool is_valid_event_line(const std::string& line, int line_number,
                           std::set<Error>& errors) const;

 private:
  const ConverterConfig& config_;
  std::unordered_set<std::string> valid_event_keywords_;
  std::unordered_set<std::string> wake_keywords_;
};

}  // namespace txt
}  // namespace validator

#endif  // VALIDATOR_TXT_RULES_LINE_RULES_H_