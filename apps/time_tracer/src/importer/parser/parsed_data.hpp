// importer/parser/parsed_data.hpp
#ifndef IMPORTER_PARSER_PARSED_DATA_H_
#define IMPORTER_PARSER_PARSED_DATA_H_

#include <unordered_set>
#include <vector>

#include "importer/model/time_sheet_data.hpp"

// 统一存放所有解析出的数据
struct ParsedData {
  std::vector<DayData> days;
  std::vector<TimeRecordInternal> records;
};

#endif  // IMPORTER_PARSER_PARSED_DATA_H_