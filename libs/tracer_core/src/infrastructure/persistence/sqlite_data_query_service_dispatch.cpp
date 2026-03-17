// infrastructure/persistence/sqlite_data_query_service_dispatch.cpp
import tracer.core.infrastructure.query.data.orchestrators;

#include <stdexcept>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"

namespace infra_data_query = tracer::core::infrastructure::query::data;
namespace infra_data_query_orchestrators =
    tracer::core::infrastructure::query::data::orchestrators;

namespace infrastructure::persistence::data_query_service_internal {

auto DispatchDataQueryAction(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    infra_data_query::DataQueryAction action,
    const infra_data_query::QueryFilters& base_filters)
    -> tracer_core::core::dto::TextOutput {
  switch (action) {
    case infra_data_query::DataQueryAction::kYears:
      return infra_data_query_orchestrators::HandleYearsQuery(
          db_conn, request.output_mode);
    case infra_data_query::DataQueryAction::kMonths:
      return infra_data_query_orchestrators::HandleMonthsQuery(
          db_conn, base_filters, request.output_mode);
    case infra_data_query::DataQueryAction::kDays:
      return infra_data_query_orchestrators::HandleDaysQuery(
          db_conn, base_filters, request.output_mode);
    case infra_data_query::DataQueryAction::kDaysDuration:
      return infra_data_query_orchestrators::HandleDaysDurationQuery(
          db_conn, base_filters, request.output_mode);
    case infra_data_query::DataQueryAction::kDaysStats:
      return infra_data_query_orchestrators::HandleDaysStatsQuery(
          db_conn, request, base_filters, request.output_mode);
    case infra_data_query::DataQueryAction::kSearch:
      return infra_data_query_orchestrators::HandleSearchQuery(
          db_conn, base_filters, request.output_mode);
    case infra_data_query::DataQueryAction::kActivitySuggest:
      return infra_data_query_orchestrators::HandleActivitySuggestQuery(
          db_conn, request, request.output_mode);
    case infra_data_query::DataQueryAction::kReportChart:
      return infra_data_query_orchestrators::HandleReportChartQuery(
          db_conn, request, request.output_mode);
    case infra_data_query::DataQueryAction::kTree:
      return infra_data_query_orchestrators::HandleTreeQuery(
          db_conn, request, base_filters, request.output_mode);
  }
  throw std::runtime_error("Unhandled data query action.");
}

}  // namespace infrastructure::persistence::data_query_service_internal
