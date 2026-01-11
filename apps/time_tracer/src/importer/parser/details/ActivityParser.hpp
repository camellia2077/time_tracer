// importer/parser/details/ActivityParser.hpp
#ifndef ACTIVITY_PARSER_HPP
#define ACTIVITY_PARSER_HPP

#include "importer/model/TimeSheetData.hpp"
#include <nlohmann/json.hpp>

class ActivityParser {
public:
    // --- [核心修改] ---
    // 移除 parent_child_pairs 参数，简化接口
    TimeRecordInternal parse(
        const nlohmann::json& activity_json,
        const std::string& date) const;
};

#endif // ACTIVITY_PARSER_HPP