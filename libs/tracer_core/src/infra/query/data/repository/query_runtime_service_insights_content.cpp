#include "infra/query/data/repository/query_runtime_service_insights_content_support.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

#include "infra/query/data/internal/request.hpp"

import tracer.core.infrastructure.query.data.orchestrators.date_range_resolver;

namespace infra_data_query_orchestrators =
    tracer::core::infrastructure::query::data::orchestrators;
namespace infra_data_query_stats =
    tracer::core::infrastructure::query::data::stats;
using tracer_core::core::dto::InsightsAverageDayBasis;

namespace tracer::core::infrastructure::query::data::internal {
namespace {

constexpr int kDefaultInsightsChartLookbackDays = 7;
constexpr int kDefaultInsightsCompositionLookbackDays = 7;

using nlohmann::json;

auto BuildCompositionTreeNodePayload(
    const infra_data_query_stats::InsightsCompositionTreeNodeView& view,
    const infra_data_query_stats::InsightsCompositionStats& stats) -> json {
  const auto kStatsIt = stats.nodes.find(view.path);
  const auto kAverageDurationSeconds =
      kStatsIt == stats.nodes.end() ? 0LL
                                    : kStatsIt->second.average_duration_seconds;
  const auto kAverageOccurrenceCount =
      kStatsIt == stats.nodes.end() ? 0.0
                                    : kStatsIt->second.average_occurrence_count;
  const auto kAverageDurationPerOccurrenceSeconds =
      kStatsIt == stats.nodes.end()
          ? 0LL
          : kStatsIt->second.average_duration_per_occurrence_seconds;
  const auto kAverageOccurrenceRatio =
      kStatsIt == stats.nodes.end() ? 0.0
                                    : kStatsIt->second.average_occurrence_ratio;
  json payload = {
      {"name", view.name},
      {"duration_seconds", view.node->duration},
      {"occurrence_count", view.node->occurrence_count},
      {"average_duration_seconds", kAverageDurationSeconds},
      {"average_duration_per_occurrence_seconds",
       kAverageDurationPerOccurrenceSeconds},
      {"average_occurrence_count", kAverageOccurrenceCount},
      {"average_occurrence_ratio", kAverageOccurrenceRatio},
      {"children", json::array()},
  };

  for (const auto& child : view.children) {
    payload["children"].push_back(
        BuildCompositionTreeNodePayload(child, stats));
  }
  return payload;
}

auto BuildInsightsChartRootNodePayload(const std::string& name,
                                       const insights::ProjectNode& node,
                                       const std::string& parent_path) -> json {
  const std::string kPath =
      parent_path.empty() ? name : parent_path + "_" + name;
  json payload = {
      {"name", name},
      {"path", kPath},
      {"duration_seconds", node.duration},
      {"children", json::array()},
  };

  std::vector<std::string> child_names;
  child_names.reserve(node.children.size());
  for (const auto& [child_name, child] : node.children) {
    static_cast<void>(child);
    child_names.push_back(child_name);
  }
  std::ranges::sort(child_names);
  for (const auto& child_name : child_names) {
    payload["children"].push_back(BuildInsightsChartRootNodePayload(
        child_name, node.children.at(child_name), kPath));
  }
  return payload;
}

auto NormalizeInsightsChartRootFilter(const std::optional<std::string>& root)
    -> std::optional<std::string> {
  if (!root.has_value()) {
    return std::nullopt;
  }
  const auto kBegin = std::find_if_not(
      root->begin(), root->end(),
      [](unsigned char character) { return std::isspace(character) != 0; });
  const auto kEnd = std::find_if_not(root->rbegin(), root->rend(),
                                     [](unsigned char character) {
                                       return std::isspace(character) != 0;
                                     })
                        .base();
  if (kBegin >= kEnd) {
    return std::nullopt;
  }
  return std::string(kBegin, kEnd);
}

}  // namespace

auto AverageDayBasisName(InsightsAverageDayBasis basis) -> std::string_view {
  return basis == InsightsAverageDayBasis::kCalendarDays ? "calendar_days"
                                                         : "active_days";
}

auto BuildCompositionTreePayload(
    const std::vector<infra_data_query_stats::InsightsCompositionTreeNodeView>&
        tree,
    const infra_data_query_stats::InsightsCompositionStats& stats) -> json {
  json payload = json::array();
  for (const auto& root : tree) {
    payload.push_back(BuildCompositionTreeNodePayload(root, stats));
  }
  return payload;
}

auto BuildInsightsChartRootTreePayload(const insights::ProjectTree& tree)
    -> json {
  json payload = json::array();
  std::vector<std::string> root_names;
  root_names.reserve(tree.size());
  for (const auto& [root_name, node] : tree) {
    static_cast<void>(node);
    root_names.push_back(root_name);
  }
  std::ranges::sort(root_names);
  for (const auto& root_name : root_names) {
    payload.push_back(
        BuildInsightsChartRootNodePayload(root_name, tree.at(root_name), ""));
  }
  return payload;
}

auto ResolveRequestedRootFilter(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> std::optional<std::string> {
  const auto kNormalizedRoot = NormalizeInsightsChartRootFilter(request.root);
  if (kNormalizedRoot.has_value()) {
    return kNormalizedRoot;
  }
  return NormalizeInsightsChartRootFilter(request.project);
}

auto ResolveInsightsQueryWindow(
    const tracer_core::core::dto::DataQueryRequest& request,
    int default_lookback_days,
    const infra_data_query_orchestrators::ExplicitDateRangeErrors&
        explicit_range_errors) -> ResolvedInsightsQueryWindow {
  const int kPayloadLookbackDays = ResolvePositiveLookbackDays(
      request.lookback_days, default_lookback_days, "--lookback-days");
  const auto kExplicitRange =
      infra_data_query_orchestrators::ResolveExplicitDateRange(
          request.from_date, request.to_date, explicit_range_errors);

  ResolvedInsightsQueryWindow window{
      .payload_lookback_days = kPayloadLookbackDays,
      .explicit_range = kExplicitRange,
      .range = kExplicitRange.value_or(
          infra_data_query_orchestrators::ResolveRollingDateRange(
              kPayloadLookbackDays)),
  };
  infra_data_query_orchestrators::ValidateDateRange(
      infra_data_query_orchestrators::DateRangeBoundaries{
          .start_date = window.range.start_date,
          .end_date = window.range.end_date,
      },
      explicit_range_errors.validation);
  return window;
}

auto ValidateInsightsChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultInsightsChartLookbackDays,
      "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "insights-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "insights-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error =
                  "insights-chart resolved invalid date range.",
          },
  };
  static_cast<void>(infra_data_query_orchestrators::ResolveExplicitDateRange(
      request.from_date, request.to_date, kRangeErrors));
}

auto ValidateInsightsCompositionRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(
      request.lookback_days, kDefaultInsightsCompositionLookbackDays,
      "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "insights-composition requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error = "insights-composition invalid range: "
                                     "from_date must be <= to_date.",
              .invalid_date_error =
                  "insights-composition resolved invalid date range.",
          },
  };
  static_cast<void>(infra_data_query_orchestrators::ResolveExplicitDateRange(
      request.from_date, request.to_date, kRangeErrors));
}

}  // namespace tracer::core::infrastructure::query::data::internal
