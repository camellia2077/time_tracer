// db_inserter/parser/pipelines/ActivityParser.hpp
#ifndef ACTIVITY_PARSER_HPP
#define ACTIVITY_PARSER_HPP

#include "db_inserter/model/time_sheet_model.hpp"
#include <nlohmann/json.hpp>
#include <unordered_set>

class ActivityParser {
public:
    TimeRecordInternal parse(
        const nlohmann::json& activity_json,
        const std::string& date,
        std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs) const;
};

#endif // ACTIVITY_PARSER_HPP