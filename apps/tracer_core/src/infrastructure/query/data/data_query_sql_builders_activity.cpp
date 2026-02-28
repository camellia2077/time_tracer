// infrastructure/query/data/data_query_sql_builders_activity.cpp
#include <string>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

constexpr size_t kActivitySuggestionsSqlReserve = 1300;

}  // namespace

auto BuildActivitySuggestionsSql(const ActivitySuggestionQueryOptions& options)
    -> std::string {
  std::string sql;
  sql.reserve(kActivitySuggestionsSqlReserve);
  sql += "WITH latest_record AS (";
  sql += "  SELECT MAX(tr.";
  sql += schema::time_records::db::kDate;
  sql += ") AS max_date FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += "), scored_records AS (";
  sql += "  SELECT tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " AS activity_name, tr.";
  sql += schema::time_records::db::kDuration;
  sql += " AS duration_seconds, tr.";
  sql += schema::time_records::db::kDate;
  sql += " AS record_date, CAST((julianday(lr.max_date) - julianday(tr.";
  sql += schema::time_records::db::kDate;
  sql += ")) AS INTEGER) AS days_ago";
  sql += "    FROM ";
  sql += schema::time_records::db::kTable;
  sql += " tr";
  sql += " CROSS JOIN latest_record lr";
  sql += "   WHERE lr.max_date IS NOT NULL";
  sql += "     AND tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " IS NOT NULL";
  sql += "     AND tr.";
  sql += schema::time_records::db::kProjectPathSnapshot;
  sql += " <> ''";
  sql += "), aggregated AS (";
  sql += "  SELECT activity_name, COUNT(*) AS usage_count,";
  sql += "         SUM(duration_seconds) AS total_duration_seconds,";
  sql += "         MAX(record_date) AS last_used_date,";
  sql += "         SUM(CAST(? - days_ago AS REAL)) AS frequency_score,";
  sql +=
      "         SUM(CAST(duration_seconds AS REAL) * CAST(? - days_ago AS "
      "REAL))";
  sql += "             AS duration_score";
  sql += "    FROM scored_records";
  sql += "   WHERE days_ago BETWEEN 0 AND (? - 1)";
  if (options.prefix.has_value() && !options.prefix->empty()) {
    sql += "     AND activity_name LIKE ? ESCAPE '\\\\'";
  }
  sql += "   GROUP BY activity_name";
  sql += ")";
  sql += " SELECT activity_name, usage_count, total_duration_seconds,";
  sql += "        last_used_date,";
  sql +=
      "        CASE WHEN ? = 1 THEN duration_score ELSE frequency_score END AS "
      "score";
  sql += "   FROM aggregated";
  sql += "  ORDER BY score DESC, usage_count DESC, activity_name ASC";
  sql += "  LIMIT ?;";

  return sql;
}

}  // namespace tracer_core::infrastructure::query::data::internal
