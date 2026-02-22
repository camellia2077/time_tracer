// infrastructure/query/data/data_query_repository_sql.hpp
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

struct sqlite3;

namespace time_tracer::infrastructure::query::data::detail {

struct SqlParam {
  enum class Type { kText, kInt };
  Type type = Type::kText;
  std::string text_value;
  int int_value = 0;
};

struct DateParts {
  int kYear = 0;
  int kMonth = 0;
  int day = 0;
};

[[nodiscard]] auto FormatDate(const DateParts& parts) -> std::string;

[[nodiscard]] auto BuildProjectDateJoinSql() -> std::string;

[[nodiscard]] auto BuildWhereClauses(const QueryFilters& filters,
                                     std::vector<SqlParam>& params)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryStringColumn(sqlite3* db_conn, const std::string& sql,
                                     const std::vector<SqlParam>& params)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryYearMonth(sqlite3* db_conn, const std::string& sql,
                                  const std::vector<SqlParam>& params)
    -> std::vector<std::pair<int, int>>;

[[nodiscard]] auto QueryRowsWithTotalDuration(
    sqlite3* db_conn, const std::string& sql,
    const std::vector<SqlParam>& params) -> std::vector<DayDurationRow>;

}  // namespace time_tracer::infrastructure::query::data::detail
