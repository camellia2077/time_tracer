// importer/parser/details/ActivityParser.hpp
#ifndef IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_
#define IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_

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

#endif // IMPORTER_PARSER_DETAILS_ACTIVITY_PARSER_HPP_