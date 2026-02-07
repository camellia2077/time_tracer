// importer/parser/details/activity_parser.hpp
#ifndef IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_H_
#define IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_H_

#include <nlohmann/json.hpp>

#include "application/importer/model/time_sheet_data.hpp"

class ActivityParser {
 public:
  static auto Parse(const nlohmann::json& activity_json) -> TimeRecordInternal;
};

#endif  // IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_H_