// infrastructure/query/data/data_query_sql_common.cpp
#include <sqlite3.h>

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

[[nodiscard]] auto EscapeLikeLiteral(const std::string& value) -> std::string {
  std::string escaped;
  escaped.reserve(value.size());
  for (const char kCharacter : value) {
    if (kCharacter == '%' || kCharacter == '_' || kCharacter == '\\') {
      escaped.push_back('\\');
    }
    escaped.push_back(kCharacter);
  }
  return escaped;
}

[[nodiscard]] auto HasProjectPathSnapshotColumn(sqlite3* db_conn) -> bool {
  const std::string kSql =
      std::format("PRAGMA table_info({});", schema::time_records::db::kTable);

  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_conn, kSql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    return false;
  }

  bool found = false;
  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(statement)) == SQLITE_ROW) {
    const unsigned char* name_text = sqlite3_column_text(statement, 1);
    if (name_text == nullptr) {
      continue;
    }
    if (std::string_view(reinterpret_cast<const char*>(name_text)) ==
        schema::time_records::db::kProjectPathSnapshot) {
      found = true;
      break;
    }
  }

  sqlite3_finalize(statement);
  return found;
}

}  // namespace

auto ClampPositiveOrDefault(int value, int fallback) -> int {
  if (value <= 0) {
    return fallback;
  }
  return value;
}

auto BuildRootPrefixLikePattern(std::string_view root) -> std::string {
  return EscapeLikeLiteral(std::string(root)) + "\\_%";
}

auto EnsureProjectPathSnapshotColumnOrThrow(sqlite3* db_conn,
                                            std::string_view query_name)
    -> void {
  if (HasProjectPathSnapshotColumn(db_conn)) {
    return;
  }
  throw std::runtime_error(std::string(query_name) +
                           " requires time_records.project_path_snapshot. "
                           "Please upgrade or rebuild the database.");
}

}  // namespace tracer_core::infrastructure::query::data::internal
