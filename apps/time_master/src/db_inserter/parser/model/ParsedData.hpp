// db_inserter/parser/model/ParsedData.hpp
#ifndef PARSED_DATA_HPP
#define PARSED_DATA_HPP

#include "db_inserter/model/time_sheet_model.hpp"
#include <vector>
#include <unordered_set>

// 统一存放所有解析出的数据
struct ParsedData {
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    // --- [核心修改] 移除不再需要的 parent_child_pairs ---
};

#endif // PARSED_DATA_HPP