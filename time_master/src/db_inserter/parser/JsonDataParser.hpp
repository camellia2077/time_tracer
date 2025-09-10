// db_inserter/parser/JsonDataParser.hpp
#ifndef JSON_DATA_PARSER_HPP
#define JSON_DATA_PARSER_HPP

#include <string>
#include <vector>
#include "db_inserter/model/time_sheet_model.hpp"
#include <nlohmann/json.hpp>

class JsonDataParser {
public:
    // 解析成功后，数据会存放在这些公共成员中
    std::vector<DayData> days;
    std::vector<TimeRecordInternal> records;
    std::unordered_set<std::pair<std::string, std::string>, pair_hash> parent_child_pairs;

    JsonDataParser() = default;

    // 主解析函数
    bool parse_file(const std::string& filename);

private:
    // 内部辅助函数
    void parse_day_object(const nlohmann::json& day_json);
    void process_activity(const nlohmann::json& activity_json, const std::string& date);
};

#endif // JSON_DATA_PARSER_HPP