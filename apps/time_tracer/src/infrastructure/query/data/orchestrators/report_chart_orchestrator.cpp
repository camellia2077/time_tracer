#include "infrastructure/query/data/orchestrators/report_chart_orchestrator.hpp"

#include <utility>

#include "infrastructure/persistence/sqlite_data_query_service_internal.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

namespace query_internal =
    infrastructure::persistence::data_query_service_internal;
namespace data_query_renderers =
    time_tracer::infrastructure::query::data::renderers;

namespace time_tracer::infrastructure::query::data::orchestrators {
namespace {

auto BuildSuccessOutput(std::string content)
    -> time_tracer::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleReportChartQuery(
    sqlite3* db_conn, const time_tracer::core::dto::DataQueryRequest& request,
    bool semantic_json) -> time_tracer::core::dto::TextOutput {
  std::string content =
      query_internal::BuildReportChartContent(db_conn, request);
  return BuildSuccessOutput(data_query_renderers::RenderJsonObjectOutput(
      "report_chart", std::move(content), semantic_json));
}

}  // namespace time_tracer::infrastructure::query::data::orchestrators
