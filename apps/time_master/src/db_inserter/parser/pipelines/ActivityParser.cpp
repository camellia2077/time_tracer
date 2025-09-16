// db_inserter/parser/pipelines/ActivityParser.cpp
#include "ActivityParser.hpp"
#include <stdexcept>

// --- [核心修改] ---
// 移除 parent_child_pairs 参数
TimeRecordInternal ActivityParser::parse(
    const nlohmann::json& activity_json,
    const std::string& date) const {
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
        
        // --- [核心修改] ---
        // 直接从 JSON 中获取 project_path，不再处理层级关系
        record.project_path = activity_details.at("project_path");
        
        return record;

    } catch (const nlohmann::json::out_of_range& e) {
        throw std::runtime_error("Required JSON key not found in activity: " + std::string(e.what()));
    }
}