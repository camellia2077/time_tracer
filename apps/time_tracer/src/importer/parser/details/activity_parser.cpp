// importer/parser/details/activity_parser.cpp
#include "activity_parser.hpp"

#include <stdexcept>

auto ActivityParser::parse(const nlohmann::json& activity_json)
    -> TimeRecordInternal {
  try {
    TimeRecordInternal record;
    record.logical_id = activity_json.at("logical_id");
    record.start_timestamp = activity_json.at("start_timestamp");
    record.end_timestamp = activity_json.at("end_timestamp");
    // The 'date' parameter was removed, so this line needs to be removed or
    // handled differently. Assuming 'date' is no longer passed and should not
    // be set here. record.date = date;

    // [核心修改] 适配字段名
    // JSON key 已经在 Output.cpp 重构中更新为 "start_time" 和 "end_time"
    // BaseActivityRecord 成员名为 start_time_str
    record.start_time_str = activity_json.at("start_time");
    record.end_time_str = activity_json.at("end_time");
    record.duration_seconds = activity_json.at("duration_seconds");

    if (activity_json.contains("activity_remark") &&
        !activity_json["activity_remark"].is_null()) {
      record.remark = activity_json["activity_remark"].get<std::string>();
    }

    const auto& activity_details = activity_json.at("activity");
    record.project_path = activity_details.at("project_path");

    return record;

  } catch (const nlohmann::json::out_of_range& e) {
    throw std::runtime_error("Required JSON key not found in activity: " +
                             std::string(e.what()));
  }
}