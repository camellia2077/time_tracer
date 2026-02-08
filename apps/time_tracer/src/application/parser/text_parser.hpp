// adapters/input/parser/text_parser.hpp
#ifndef ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_
#define ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_

#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>

#include "domain/model/daily_log.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

class TextParser {
 public:
  explicit TextParser(const ConverterConfig& config);
  auto Parse(std::istream& input_stream,
             std::function<void(DailyLog&)> on_new_day,
             std::string_view source_file) -> void;

 private:
  const ConverterConfig& config_;

  std::string year_prefix_;
  const std::vector<std::string>&
      wake_keywords_;  // [优化] 直接引用 vector，避免拷贝 set

  static auto IsYearMarker(const std::string& line) -> bool;
  static auto IsNewDayMarker(const std::string& line) -> bool;
  auto ParseLine(const std::string& line, int line_number,
                 DailyLog& current_day, std::string_view source_file) const
      -> void;

  struct RemarkResult {
    std::string description;
    std::string remark;
  };

  struct EventInput {
    std::string_view description;
    std::string_view time_str_hhmm;
  };

  [[nodiscard]] static auto ExtractRemark(std::string_view remaining_line)
      -> RemarkResult;

  auto ProcessEventContext(DailyLog& current_day, EventInput input) const
      -> bool;
};

#endif  // ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_
