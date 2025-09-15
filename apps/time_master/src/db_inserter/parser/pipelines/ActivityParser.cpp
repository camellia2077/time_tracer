// db_inserter/parser/pipelines/ActivityParser.cpp
#include "ActivityParser.hpp"
#include <stdexcept>

TimeRecordInternal ActivityParser::parse(
    const nlohmann::json& activity_json,
    const std::string& date,
    std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs) const {
    try {
        TimeRecordInternal record;
        // ... (other fields are unchanged) ...
        record.logical_id = activity_json.at("logicalId");
        record.start_timestamp = activity_json.at("startTimestamp");
        record.end_timestamp = activity_json.at("endTimestamp");
        record.date = date;
        record.start = activity_json.at("startTime");
        record.end = activity_json.at("endTime");
        record.duration_seconds = activity_json.at("durationSeconds");

        if (activity_json.contains("activityRemark") && !activity_json["activityRemark"].is_null()) {
            record.activityRemark = activity_json["activityRemark"].get<std::string>();
        }

        const auto& activity_details = activity_json.at("activity");
        // [核心修改]
        std::string parent_name = activity_details.at("parent");
        std::string project_path = parent_name;

        // [核心修改]
        if (activity_details.contains("children") && activity_details["children"].is_array()) {
            for (const auto& child_json : activity_details["children"]) {
                std::string child_name = child_json.get<std::string>();
                std::string current_parent_path = project_path;
                project_path += "_" + child_name;
                // 这里的逻辑依然是 child, parent，非常清晰
                parent_child_pairs.insert({project_path, current_parent_path});
            }
        }

        record.project_path = project_path;
        return record;

    } catch (const nlohmann::json::out_of_range& e) {
        throw std::runtime_error("Required JSON key not found in activity: " + std::string(e.what()));
    }
}