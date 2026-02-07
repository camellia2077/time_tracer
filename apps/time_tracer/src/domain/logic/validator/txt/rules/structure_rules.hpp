// domain/logic/validator/txt/rules/structure_rules.hpp
#ifndef VALIDATOR_TXT_RULES_STRUCTURE_RULES_H_
#define VALIDATOR_TXT_RULES_STRUCTURE_RULES_H_

#include <set>
#include <string>

#include "domain/logic/validator/common/validator_utils.hpp"

namespace validator {
namespace txt {

class StructureRules {
 public:
  StructureRules() = default;

  // [新增] 重置状态，用于处理新文件
  void reset();

  void process_year_line(int line_number, const std::string& line,
                         std::set<Error>& errors);
  void process_date_line(int line_number, const std::string& line,
                         std::set<Error>& errors);
  void process_remark_line(int line_number, const std::string& line,
                           std::set<Error>& errors) const;
  void process_event_line(int line_number, const std::string& line,
                          std::set<Error>& errors);
  static void process_unrecognized_line(int line_number,
                                        const std::string& line,
                                        std::set<Error>& errors);

  bool has_seen_year() const;

 private:
  bool has_seen_year_ = false;
  bool has_seen_date_in_block_ = false;
  bool has_seen_event_in_day_ = false;

  // [新增] 用于检查"文件的第一天必须是1号"
  bool has_seen_any_date_ = false;

  int last_seen_year_ = 0;
};

}  // namespace txt
}  // namespace validator

#endif  // VALIDATOR_TXT_RULES_STRUCTURE_RULES_H_