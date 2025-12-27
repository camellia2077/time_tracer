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
        record.logical_id = activity_json.at("logical_id");
        record.start_timestamp = activity_json.at("start_timestamp");
        record.end_timestamp = activity_json.at("end_timestamp");
        record.date = date;
        record.start = activity_json.at("start_time");
        record.end = activity_json.at("end_time");
        record.duration_seconds = activity_json.at("duration_seconds");

        if (activity_json.contains("activity_remark") && !activity_json["activity_remark"].is_null()) {
            record.activity_remark = activity_json["activity_remark"].get<std::string>(); // MODIFIED
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