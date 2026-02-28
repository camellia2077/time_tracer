// infrastructure/query/data/data_query_sql_filters.cpp
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/query/data/data_query_repository_internal.hpp"
#include "infrastructure/schema/day_schema.hpp"

namespace tracer_core::infrastructure::query::data::internal {
namespace {

constexpr size_t kDayClauseReserve = 24;

auto AddIntPredicateClause(std::vector<std::string>& clauses,
                           std::string_view column_name, int value,
                           std::vector<detail::SqlParam>& params) -> void {
  std::string clause;
  clause.reserve(kDayClauseReserve);
  clause += column_name;
  clause += " = ?";
  clauses.push_back(std::move(clause));
  params.push_back({.type = detail::SqlParam::Type::kInt,
                    .text_value = "",
                    .int_value = value});
}

auto AddTextPredicateClause(std::vector<std::string>& clauses,
                            std::string_view column_name,
                            std::string_view comparison_operator,
                            const std::string& value,
                            std::vector<detail::SqlParam>& params) -> void {
  std::string clause;
  clause.reserve(kDayClauseReserve);
  clause += column_name;
  clause += " ";
  clause += comparison_operator;
  clause += " ?";
  clauses.push_back(std::move(clause));
  params.push_back({.type = detail::SqlParam::Type::kText,
                    .text_value = value,
                    .int_value = 0});
}

}  // namespace

auto NeedRecordsJoinForFilters(const QueryFilters& filters) -> bool {
  return filters.project.has_value() || filters.root.has_value() ||
         filters.remark.has_value();
}

auto BuildDayFilterClauses(const std::optional<int>& year,
                           const std::optional<int>& month,
                           const std::optional<std::string>& from_date,
                           const std::optional<std::string>& to_date,
                           std::vector<detail::SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> clauses;
  if (year.has_value()) {
    AddIntPredicateClause(clauses, schema::day::db::kYear, *year, params);
  }
  if (month.has_value()) {
    AddIntPredicateClause(clauses, schema::day::db::kMonth, *month, params);
  }
  if (from_date.has_value()) {
    AddTextPredicateClause(clauses, schema::day::db::kDate, ">=", *from_date,
                           params);
  }
  if (to_date.has_value()) {
    AddTextPredicateClause(clauses, schema::day::db::kDate, "<=", *to_date,
                           params);
  }
  return clauses;
}

auto AppendWhereClauses(std::string& sql,
                        const std::vector<std::string>& clauses) -> void {
  if (clauses.empty()) {
    return;
  }

  sql += " WHERE " + clauses[0];
  for (size_t index = 1; index < clauses.size(); ++index) {
    sql += " AND " + clauses[index];
  }
}

auto AppendOptionalLimitClause(std::string& sql,
                               const std::optional<int>& limit,
                               std::vector<detail::SqlParam>& params) -> void {
  if (!limit.has_value() || *limit <= 0) {
    return;
  }

  sql += " LIMIT ?";
  params.push_back({.type = detail::SqlParam::Type::kInt,
                    .text_value = "",
                    .int_value = *limit});
}

}  // namespace tracer_core::infrastructure::query::data::internal
