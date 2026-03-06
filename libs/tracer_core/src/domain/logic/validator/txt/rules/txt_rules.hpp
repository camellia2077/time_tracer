// domain/logic/validator/txt/rules/txt_rules.hpp
#ifndef DOMAIN_LOGIC_VALIDATOR_TXT_RULES_TXT_RULES_H_
#define DOMAIN_LOGIC_VALIDATOR_TXT_RULES_TXT_RULES_H_

#include <optional>
#include <set>
#include <string>
#include <unordered_set>

#include "domain/logic/validator/common/validator_utils.hpp"
#include "domain/model/source_span.hpp"
#include "domain/types/converter_config.hpp"

namespace validator::txt {

class LineRules {
 public:
  explicit LineRules(const ConverterConfig& config);

  static auto IsYear(const std::string& line) -> bool;
  static auto IsMonth(const std::string& line) -> bool;
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

class StructureRules {
 public:
  StructureRules() = default;
  void Reset();

  void ProcessYearLine(int line_number, const std::string& line,
                       std::set<Error>& errors, const SourceSpan& span);
  void ProcessMonthLine(int line_number, const std::string& line,
                        std::set<Error>& errors, const SourceSpan& span);
  void ProcessDateLine(int line_number, const std::string& line,
                       std::set<Error>& errors, const SourceSpan& span);
  void ProcessRemarkLine(int line_number, const std::string& line,
                         std::set<Error>& errors, const SourceSpan& span) const;
  void ProcessEventLine(int line_number, const std::string& line,
                        std::set<Error>& errors, const SourceSpan& span);
  static void ProcessUnrecognizedLine(int line_number, const std::string& line,
                                      std::set<Error>& errors,
                                      const SourceSpan& span);

  [[nodiscard]] auto HasSeenYear() const -> bool;
  [[nodiscard]] auto HasSeenMonth() const -> bool;

 private:
  bool has_seen_year_ = false;
  bool has_seen_date_in_block_ = false;
  bool has_seen_event_in_day_ = false;
  bool has_seen_any_date_ = false;
  bool has_seen_month_ = false;
  bool has_reported_missing_month_header_ = false;
  std::string month_header_;
  int last_seen_year_ = 0;
};

}  // namespace validator::txt

#endif  // DOMAIN_LOGIC_VALIDATOR_TXT_RULES_TXT_RULES_H_
