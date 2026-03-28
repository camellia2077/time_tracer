// infra/schema/sqlite_schema.hpp
#ifndef INFRASTRUCTURE_SCHEMA_SQLITE_SCHEMA_H_
#define INFRASTRUCTURE_SCHEMA_SQLITE_SCHEMA_H_

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
inline constexpr std::string_view kProjectPathSnapshot =
    "project_path_snapshot";
inline constexpr std::string_view kActivityRemark = "activity_remark";
}  // namespace schema::time_records::db

namespace schema::projects::db {
inline constexpr std::string_view kTable = "projects";
inline constexpr std::string_view kId = "id";
inline constexpr std::string_view kName = "name";
inline constexpr std::string_view kParentId = "parent_id";
inline constexpr std::string_view kFullPath = "full_path";
inline constexpr std::string_view kDepth = "depth";
}  // namespace schema::projects::db

namespace schema::projects::cte {
inline constexpr std::string_view kProjectPaths = "project_paths";
inline constexpr std::string_view kPath = "path";
}  // namespace schema::projects::cte

namespace schema::ingest_month_sync::db {
inline constexpr std::string_view kTable = "ingest_month_sync";
inline constexpr std::string_view kMonthKey = "month_key";
inline constexpr std::string_view kTxtRelativePath = "txt_relative_path";
inline constexpr std::string_view kTxtContentHashSha256 =
    "txt_content_hash_sha256";
inline constexpr std::string_view kIngestedAtUnixMs = "ingested_at_unix_ms";
}  // namespace schema::ingest_month_sync::db

#endif  // INFRASTRUCTURE_SCHEMA_SQLITE_SCHEMA_H_
