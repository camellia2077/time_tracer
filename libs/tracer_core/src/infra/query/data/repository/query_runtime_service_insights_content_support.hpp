#ifndef INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INSIGHTS_CONTENT_SUPPORT_HPP_
#define INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INSIGHTS_CONTENT_SUPPORT_HPP_

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "application/dto/query_requests.hpp"
#include "domain/insights/models/project_tree.hpp"
#include "infra/query/data/orchestrators/date_range_resolver.hpp"
#include "infra/query/data/stats/insights_chart_stats_calculator.hpp"

namespace tracer::core::infrastructure::query::data::internal {

using InsightsQueryJson = nlohmann::json;

struct ResolvedInsightsQueryWindow {
  int payload_lookback_days = 0;
  std::optional<orchestrators::ResolvedDateRange> explicit_range;
  orchestrators::ResolvedDateRange range;
};

[[nodiscard]] auto AverageDayBasisName(
    tracer_core::core::dto::InsightsAverageDayBasis basis) -> std::string_view;

[[nodiscard]] auto ResolveRequestedRootFilter(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> std::optional<std::string>;

[[nodiscard]] auto ResolveInsightsQueryWindow(
    const tracer_core::core::dto::DataQueryRequest& request,
    int default_lookback_days,
    const orchestrators::ExplicitDateRangeErrors& explicit_range_errors)
    -> ResolvedInsightsQueryWindow;

[[nodiscard]] auto BuildCompositionTreePayload(
    const std::vector<stats::InsightsCompositionTreeNodeView>& tree,
    const stats::InsightsCompositionStats& composition_stats)
    -> InsightsQueryJson;

[[nodiscard]] auto BuildInsightsChartRootTreePayload(
    const insights::ProjectTree& tree) -> InsightsQueryJson;

}  // namespace tracer::core::infrastructure::query::data::internal

#endif  // INFRASTRUCTURE_QUERY_DATA_REPOSITORY_QUERY_RUNTIME_SERVICE_INSIGHTS_CONTENT_SUPPORT_HPP_
