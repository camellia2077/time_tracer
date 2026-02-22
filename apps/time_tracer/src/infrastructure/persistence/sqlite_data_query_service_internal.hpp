#ifndef INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_
#define INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_

#include <sqlite3.h>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include "application/ports/i_data_query_service.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_types.hpp"

class DBManager;

namespace infrastructure::persistence::data_query_service_internal {

auto TrimCopy(std::string_view value) -> std::string;

auto NormalizeProjectRootFilter(const std::optional<std::string>& project)
    -> std::optional<std::string>;

auto ParseIsoDate(std::string_view value)
    -> std::optional<std::chrono::year_month_day>;

auto FormatIsoDate(const std::chrono::year_month_day& ymd) -> std::string;

[[nodiscard]] auto ResolveCurrentSystemLocalDate()
    -> std::chrono::year_month_day;

auto ResolvePositiveLookbackDays(const std::optional<int>& candidate,
                                 int fallback, std::string_view field_name)
    -> int;

auto NormalizeBoundaryDate(std::string_view input, bool is_end) -> std::string;

auto EnsureDbConnectionOrThrow(DBManager& db_manager,
                               const std::filesystem::path& db_path)
    -> sqlite3*;

auto ToCliDataQueryAction(time_tracer::core::dto::DataQueryAction action)
    -> time_tracer::infrastructure::query::data::DataQueryAction;

auto BuildCliFilters(const time_tracer::core::dto::DataQueryRequest& request)
    -> time_tracer::infrastructure::query::data::QueryFilters;

auto BuildMappingNamesContent(
    const std::optional<std::filesystem::path>& converter_config_toml_path)
    -> std::string;

auto BuildReportChartContent(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request)
    -> std::string;

auto ApplyTreePeriod(
    const time_tracer::core::dto::DataQueryRequest& request, sqlite3* db_conn,
    time_tracer::infrastructure::query::data::QueryFilters& filters) -> void;

auto DispatchDataQueryAction(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request,
    time_tracer::infrastructure::query::data::DataQueryAction action,
    const time_tracer::infrastructure::query::data::QueryFilters& base_filters)
    -> time_tracer::core::dto::TextOutput;

}  // namespace infrastructure::persistence::data_query_service_internal

#endif  // INFRASTRUCTURE_PERSISTENCE_SQLITE_DATA_QUERY_SERVICE_INTERNAL_HPP_
