// importer/parser/ParsedData.hpp
#ifndef IMPORTER_PARSER_PARSED_DATA_HPP_
#define IMPORTER_PARSER_PARSED_DATA_HPP_

#include <vector>
#include <unordered_set>

#include "importer/model/TimeSheetData.hpp"

// 统一存放所有解析出的数据
struct ParsedData {
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
};

#endif // IMPORTER_PARSER_PARSED_DATA_HPP_