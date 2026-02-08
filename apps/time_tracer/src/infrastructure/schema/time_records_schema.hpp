// infrastructure/schema/time_records_schema.hpp
#ifndef INFRASTRUCTURE_SCHEMA_TIME_RECORDS_SCHEMA_H_
#define INFRASTRUCTURE_SCHEMA_TIME_RECORDS_SCHEMA_H_

#include <string_view>

namespace schema::time_records::db {
inline constexpr std::string_view kTable = "time_records";
inline constexpr std::string_view kLogicalId = "logical_id";
inline constexpr std::string_view kStartTimestamp = "start_timestamp";
inline constexpr std::string_view kEndTimestamp = "end_timestamp";
inline constexpr std::string_view kDate = "date";
inline constexpr std::string_view kStart = "start";
inline constexpr std::string_view kEnd = "end";
inline constexpr std::string_view kProjectId = "project_id";
inline constexpr std::string_view kDuration = "duration";
inline constexpr std::string_view kActivityRemark = "activity_remark";
}  // namespace schema::time_records::db

namespace schema::projects::db {
inline constexpr std::string_view kTable = "projects";
inline constexpr std::string_view kId = "id";
inline constexpr std::string_view kName = "name";
inline constexpr std::string_view kParentId = "parent_id";
}  // namespace schema::projects::db

namespace schema::projects::cte {
inline constexpr std::string_view kProjectPaths = "project_paths";
inline constexpr std::string_view kPath = "path";
}  // namespace schema::projects::cte

#endif  // INFRASTRUCTURE_SCHEMA_TIME_RECORDS_SCHEMA_H_
