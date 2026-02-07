// application/parser/day_parser.hpp
#ifndef APPLICATION_PARSER_DAY_PARSER_H_
#define APPLICATION_PARSER_DAY_PARSER_H_

#include <nlohmann/json.hpp>

#include "application/importer/model/time_sheet_data.hpp"

class DayParser {
 public:
  static auto Parse(const nlohmann::json& day_json) -> DayData;
};

#endif  // APPLICATION_PARSER_DAY_PARSER_H_
