// api/cli/impl/commands/query/data_query_repository.hpp
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "api/cli/impl/commands/query/data_query_types.hpp"
#include "api/cli/impl/commands/query/query_filters.hpp"

struct sqlite3;

namespace time_tracer::cli::impl::commands::query::data {

[[nodiscard]] auto QueryYears(sqlite3* db_conn) -> std::vector<std::string>;

[[nodiscard]] auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
                             const std::optional<int>& month,
                             const std::optional<std::string>& from_date,
                             const std::optional<std::string>& to_date,
                             bool reverse, const std::optional<int>& limit)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDatesByFilters(sqlite3* db_conn,
                                       const QueryFilters& filters)
    -> std::vector<std::string>;

[[nodiscard]] auto QueryDayDurations(sqlite3* db_conn,
                                     const QueryFilters& filters)
    -> std::vector<DayDurationRow>;

}  // namespace time_tracer::cli::impl::commands::query::data
