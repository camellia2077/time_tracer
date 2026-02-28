// infrastructure/query/data/data_query_row_mappers.cpp
#include <sqlite3.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

constexpr int kDurationScoreMode = 1;

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

auto PrepareStatementOrThrow(sqlite3* db_conn, const std::string& sql)
    -> sqlite3_stmt* {
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &statement, nullptr) !=
      SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }
  return statement;
}

auto BindSqlParams(sqlite3_stmt* statement,
                   const std::vector<detail::SqlParam>& params) -> void {
  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == detail::SqlParam::Type::kInt) {
      sqlite3_bind_int(statement, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(statement, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }
}

auto BindActivitySuggestions(sqlite3_stmt* statement,
                             const ActivitySuggestionQueryOptions& options,
                             int lookback_days, int limit) -> void {
  int bind_index = 1;
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  sqlite3_bind_int(statement, bind_index++, lookback_days);
  if (options.prefix.has_value() && !options.prefix->empty()) {
    std::string like_pattern = EscapeLikeLiteral(*options.prefix) + "%";
    sqlite3_bind_text(statement, bind_index++, like_pattern.c_str(), -1,
                      SQLITE_TRANSIENT);
  }
  sqlite3_bind_int(statement, bind_index++,
                   options.score_by_duration ? kDurationScoreMode : 0);
  sqlite3_bind_int(statement, bind_index, limit);
}

[[nodiscard]] auto ReadActivitySuggestionRows(sqlite3* db_conn,
                                              sqlite3_stmt* statement)
    -> std::vector<ActivitySuggestionRow> {
  std::vector<ActivitySuggestionRow> rows;

  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(statement)) == SQLITE_ROW) {
    ActivitySuggestionRow row;
    if (const unsigned char* text = sqlite3_column_text(statement, 0);
        text != nullptr) {
      row.activity_name = reinterpret_cast<const char*>(text);
    }
    row.usage_count = sqlite3_column_int(statement, 1);
    row.total_duration_seconds = sqlite3_column_int64(statement, 2);
    if (const unsigned char* text = sqlite3_column_text(statement, 3);
        text != nullptr) {
      row.last_used_date = reinterpret_cast<const char*>(text);
    }
    row.score = sqlite3_column_double(statement, 4);
    rows.push_back(std::move(row));
  }

  if (step_result != SQLITE_DONE) {
    throw std::runtime_error("Failed to execute query: " +
                             std::string(sqlite3_errmsg(db_conn)));
  }

  return rows;
}

[[nodiscard]] auto ReadProjectTreeRecords(sqlite3* db_conn,
                                          sqlite3_stmt* statement)
    -> std::vector<std::pair<std::string, long long>> {
  std::vector<std::pair<std::string, long long>> records;

  int step_result = SQLITE_OK;
  while ((step_result = sqlite3_step(statement)) == SQLITE_ROW) {
    const unsigned char* path_text = sqlite3_column_text(statement, 0);
    if (path_text == nullptr) {
      continue;
    }
    std::string path = reinterpret_cast<const char*>(path_text);
    const long long kDurationSeconds = sqlite3_column_int64(statement, 1);
    records.emplace_back(std::move(path), kDurationSeconds);
  }

  if (step_result != SQLITE_DONE) {
    throw std::runtime_error("Failed to execute query: " +
                             std::string(sqlite3_errmsg(db_conn)));
  }

  return records;
}

}  // namespace

auto ExecuteActivitySuggestions(sqlite3* db_conn, const std::string& sql,
                                const ActivitySuggestionQueryOptions& options,
                                int lookback_days, int limit)
    -> std::vector<ActivitySuggestionRow> {
  sqlite3_stmt* statement = PrepareStatementOrThrow(db_conn, sql);
  try {
    BindActivitySuggestions(statement, options, lookback_days, limit);
    std::vector<ActivitySuggestionRow> rows =
        ReadActivitySuggestionRows(db_conn, statement);
    sqlite3_finalize(statement);
    return rows;
  } catch (...) {
    sqlite3_finalize(statement);
    throw;
  }
}

auto ExecuteProjectTreeRecords(sqlite3* db_conn, const std::string& sql,
                               const std::vector<detail::SqlParam>& params)
    -> std::vector<std::pair<std::string, long long>> {
  sqlite3_stmt* statement = PrepareStatementOrThrow(db_conn, sql);
  try {
    BindSqlParams(statement, params);
    std::vector<std::pair<std::string, long long>> records =
        ReadProjectTreeRecords(db_conn, statement);
    sqlite3_finalize(statement);
    return records;
  } catch (...) {
    sqlite3_finalize(statement);
    throw;
  }
}

}  // namespace tracer_core::infrastructure::query::data::internal
