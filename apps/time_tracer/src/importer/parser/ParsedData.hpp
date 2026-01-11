// importer/parser/ParsedData.hpp
#ifndef PARSED_DATA_HPP
#define PARSED_DATA_HPP

#include <vector>
#include <unordered_set>

#include "importer/model/TimeSheetData.hpp"

// 统一存放所有解析出的数据
struct ParsedData {
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
};

#endif // PARSED_DATA_HPP