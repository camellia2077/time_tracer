// application/importer/parser/parsed_data.hpp
#ifndef APPLICATION_IMPORTER_PARSER_PARSED_DATA_H_
#define APPLICATION_IMPORTER_PARSER_PARSED_DATA_H_

#include <vector>

#include "application/importer/model/time_sheet_data.hpp"

struct ParsedData {
  std::vector<DayData> days;
  std::vector<TimeRecordInternal> records;
};

#endif  // APPLICATION_IMPORTER_PARSER_PARSED_DATA_H_
