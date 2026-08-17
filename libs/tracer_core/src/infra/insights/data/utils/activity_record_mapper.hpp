// infra/insights/data/utils/activity_record_mapper.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DATA_UTILS_ACTIVITY_RECORD_MAPPER_H_
#define INFRASTRUCTURE_INSIGHTS_DATA_UTILS_ACTIVITY_RECORD_MAPPER_H_

#include <sqlite3.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/interfaces/i_project_info_provider.hpp"
#include "domain/insights/models/daily_insights_data.hpp"

namespace tracer::core::infrastructure::insights::data::record_mapping {

struct TimeRecordColumnIndexes {
  int start_time;
  int end_time;
  int project_id;
  int duration;
  int activity_remark;
  int logical_id;
  int record_kind;
};

inline auto JoinProjectPath(const std::vector<std::string>& parts)
    -> std::string {
  if (parts.empty()) {
    return "";
  }
  std::string path = parts.front();
  for (size_t index = 1; index < parts.size(); ++index) {
    path += "_" + parts[index];
  }
  return path;
}

inline auto ParseRecordKind(const unsigned char* value) -> ActivityRecordKind {
  if (value != nullptr &&
      std::string_view(reinterpret_cast<const char*>(value)) == "end_only") {
    return ActivityRecordKind::kEndOnly;
  }
  return ActivityRecordKind::kInterval;
}

inline auto ReadTimeRecord(sqlite3_stmt* stmt,
                           const TimeRecordColumnIndexes& columns,
                           const IProjectInfoProvider& provider) -> TimeRecord {
  const auto text_at = [stmt](int index) -> std::string {
    const auto* value = sqlite3_column_text(stmt, index);
    return value == nullptr ? "" : reinterpret_cast<const char*>(value);
  };

  TimeRecord record;
  record.start_time = text_at(columns.start_time);
  record.end_time = text_at(columns.end_time);
  record.project_path = JoinProjectPath(
      provider.GetPathParts(sqlite3_column_int64(stmt, columns.project_id)));
  record.duration_seconds = sqlite3_column_int64(stmt, columns.duration);
  const auto* remark = sqlite3_column_text(stmt, columns.activity_remark);
  if (remark != nullptr) {
    record.activityRemark = reinterpret_cast<const char*>(remark);
  }
  record.logical_id = sqlite3_column_int64(stmt, columns.logical_id);
  record.kind = ParseRecordKind(sqlite3_column_text(stmt, columns.record_kind));
  return record;
}

}  // namespace tracer::core::infrastructure::insights::data::record_mapping

#endif  // INFRASTRUCTURE_INSIGHTS_DATA_UTILS_ACTIVITY_RECORD_MAPPER_H_
