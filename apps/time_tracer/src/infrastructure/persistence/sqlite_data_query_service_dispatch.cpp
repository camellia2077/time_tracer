#include <stdexcept>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/orchestrators/days_stats_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp"
#include "infrastructure/query/data/orchestrators/tree_orchestrator.hpp"

namespace infra_data_query = time_tracer::infrastructure::query::data;
namespace infra_data_query_orchestrators =
    time_tracer::infrastructure::query::data::orchestrators;

namespace infrastructure::persistence::data_query_service_internal {
namespace {

using time_tracer::core::dto::DataQueryOutputMode;

[[nodiscard]] auto IsSemanticJsonOutput(
    const time_tracer::core::dto::DataQueryRequest& request) -> bool {
  return request.output_mode == DataQueryOutputMode::kSemanticJson;
}

}  // namespace

auto DispatchDataQueryAction(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request,
    infra_data_query::DataQueryAction action,
    const infra_data_query::QueryFilters& base_filters)
    -> time_tracer::core::dto::TextOutput {
  const bool kSemanticJsonOutput = IsSemanticJsonOutput(request);
  switch (action) {
    case infra_data_query::DataQueryAction::kYears:
      return infra_data_query_orchestrators::HandleYearsQuery(
          db_conn, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kMonths:
      return infra_data_query_orchestrators::HandleMonthsQuery(
          db_conn, base_filters, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kDays:
      return infra_data_query_orchestrators::HandleDaysQuery(
          db_conn, base_filters, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kDaysDuration:
      return infra_data_query_orchestrators::HandleDaysDurationQuery(
          db_conn, base_filters, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kDaysStats:
      return infra_data_query_orchestrators::HandleDaysStatsQuery(
          db_conn, request, base_filters, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kSearch:
      return infra_data_query_orchestrators::HandleSearchQuery(
          db_conn, base_filters, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kActivitySuggest:
      return infra_data_query_orchestrators::HandleActivitySuggestQuery(
          db_conn, request, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kReportChart:
      return infra_data_query_orchestrators::HandleReportChartQuery(
          db_conn, request, kSemanticJsonOutput);
    case infra_data_query::DataQueryAction::kTree:
      return infra_data_query_orchestrators::HandleTreeQuery(
          db_conn, request, base_filters, kSemanticJsonOutput);
  }
  throw std::runtime_error("Unhandled data query action.");
}

}  // namespace infrastructure::persistence::data_query_service_internal
