// importer/parser/details/day_parser.hpp
#ifndef IMPORTER_PARSER_DETAILS_DAY_PARSER_H_
#define IMPORTER_PARSER_DETAILS_DAY_PARSER_H_

#include <nlohmann/json.hpp>

#include "importer/model/time_sheet_data.hpp"

class DayParser {
 public:
  static DayData parse(const nlohmann::json& day_json);
};

#endif  // IMPORTER_PARSER_DETAILS_DAY_PARSER_H_