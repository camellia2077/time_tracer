// adapters/input/parser/text_parser.hpp
#ifndef ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_
#define ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_

#include <functional>
#include <iostream>
#include <string>
#include <unordered_set>

#include "common/config/models/converter_config_models.hpp"
#include "domain/model/daily_log.hpp"

class TextParser {
 public:
  explicit TextParser(const ConverterConfig& config);
  void parse(std::istream& input_stream,
             std::function<void(DailyLog&)> on_new_day);

 private:
  const ConverterConfig& config_;

  std::string year_prefix_;
  const std::vector<std::string>&
      wake_keywords_;  // [优化] 直接引用 vector，避免拷贝 set

  static bool isYearMarker(const std::string& line);
  static bool isNewDayMarker(const std::string& line);
  void parseLine(const std::string& line, int line_number,
                 DailyLog& current_day) const;
};

#endif  // ADAPTERS_INPUT_PARSER_TEXT_PARSER_H_
