// db_inserter/parser/pipelines/ActivityParser.cpp
#include "ActivityParser.hpp"
#include <stdexcept>

TimeRecordInternal ActivityParser::parse(
    const nlohmann::json& activity_json,
    const std::string& date,
    std::unordered_set<std::pair<std::string, std::string>, pair_hash>& parent_child_pairs) const {
    try {
        TimeRecordInternal record;
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
        std::string title = activity_details.at("topParent");
        std::string project_path = title;

        if (activity_details.contains("parents") && activity_details["parents"].is_array()) {
            for (const auto& parent_json : activity_details["parents"]) {
                std::string parent_name = parent_json.get<std::string>();
                std::string parent_path = project_path;
                project_path += "_" + parent_name;
                parent_child_pairs.insert({project_path, parent_path});
            }
        }

        record.project_path = project_path;
        return record;

    } catch (const nlohmann::json::out_of_range& e) {
        throw std::runtime_error("Required JSON key not found in activity: " + std::string(e.what()));
    }
}