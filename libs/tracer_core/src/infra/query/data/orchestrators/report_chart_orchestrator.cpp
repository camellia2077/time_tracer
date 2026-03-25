// infra/query/data/orchestrators/report_chart_orchestrator.cpp
#include "infra/query/data/orchestrators/report_chart_orchestrator.hpp"

#include <utility>

import tracer.core.infrastructure.query.data.internal.report_mapping;
import tracer.core.infrastructure.query.data.renderers;

namespace query_internal = tracer::core::infrastructure::query::data::internal;
namespace data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;

namespace tracer::core::infrastructure::query::data::orchestrators {
namespace {

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleReportChartQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  std::string content =
      query_internal::BuildReportChartContent(db_conn, request);
  return BuildSuccessOutput(data_query_renderers::RenderJsonObjectOutput(
      "report_chart", std::move(content), output_mode));
}

}  // namespace tracer::core::infrastructure::query::data::orchestrators
