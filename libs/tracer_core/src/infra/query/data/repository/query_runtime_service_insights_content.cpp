#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "domain/insights/models/project_tree.hpp"
#include "infra/query/data/internal/insights_mapping.hpp"
#include "infra/query/data/stats/insights_chart_stats_calculator.hpp"

import tracer.core.infrastructure.query.data.orchestrators.date_range_resolver;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

namespace infra_data_query = tracer::core::infrastructure::query::data;
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

auto AverageDayBasisName(InsightsAverageDayBasis basis) -> std::string_view {
  return basis == InsightsAverageDayBasis::kCalendarDays ? "calendar_days"
                                                       : "active_days";
}

struct ResolvedInsightsQueryWindow {
  int payload_lookback_days = 0;
  std::optional<infra_data_query_orchestrators::ResolvedDateRange>
      explicit_range;
  infra_data_query_orchestrators::ResolvedDateRange range;
};

[[nodiscard]] auto BuildCompositionTreeNodePayload(
    const infra_data_query_stats::InsightsCompositionTreeNodeView& view,
    const infra_data_query_stats::InsightsCompositionStats& stats) -> json {
  const auto kStatsIt = stats.nodes.find(view.path);
  const auto kAverageDurationSeconds =
      kStatsIt == stats.nodes.end() ? 0LL
                                    : kStatsIt->second.average_duration_seconds;
  const auto kAverageOccurrenceCount =
      kStatsIt == stats.nodes.end() ? 0.0
                                    : kStatsIt->second.average_occurrence_count;
  const auto kAverageOccurrenceRatio =
      kStatsIt == stats.nodes.end() ? 0.0
                                    : kStatsIt->second.average_occurrence_ratio;
  json payload = {
      {"name", view.name},
      {"duration_seconds", view.node->duration},
      {"occurrence_count", view.node->occurrence_count},
      {"average_duration_seconds", kAverageDurationSeconds},
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

[[nodiscard]] auto BuildCompositionTreePayload(
    const std::vector<infra_data_query_stats::InsightsCompositionTreeNodeView>&
        tree,
    const infra_data_query_stats::InsightsCompositionStats& stats) -> json {
  json payload = json::array();
  for (const auto& root : tree) {
    payload.push_back(BuildCompositionTreeNodePayload(root, stats));
  }
  return payload;
}

auto ResolveRequestedRootFilter(
    const tracer_core::core::dto::DataQueryRequest& request)
    -> std::optional<std::string> {
  const auto kNormalizedRoot = NormalizeProjectRootFilter(request.root);
  if (kNormalizedRoot.has_value()) {
    return kNormalizedRoot;
  }
  return NormalizeProjectRootFilter(request.project);
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

}  // namespace

auto ValidateInsightsChartRequest(
    const tracer_core::core::dto::DataQueryRequest& request) -> void {
  static_cast<void>(ResolvePositiveLookbackDays(request.lookback_days,
                                                kDefaultInsightsChartLookbackDays,
                                                "--lookback-days"));

  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "insights-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "insights-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error = "insights-chart resolved invalid date range.",
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

auto BuildInsightsChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  const auto kSelectedRoot = ResolveRequestedRootFilter(request);
  const std::vector<std::string> kRoots =
      infra_data_query::QueryProjectRootNames(db_conn);

  ValidateInsightsChartRequest(request);
  const infra_data_query_orchestrators::ExplicitDateRangeErrors kRangeErrors{
      .missing_boundary_error =
          "insights-chart requires both --from-date and --to-date.",
      .validation =
          {
              .invalid_range_error =
                  "insights-chart invalid range: from_date must be <= to_date.",
              .invalid_date_error = "insights-chart resolved invalid date range.",
          },
  };
  const auto kWindow = ResolveInsightsQueryWindow(
      request, kDefaultInsightsChartLookbackDays, kRangeErrors);

  json payload = json::object();
  payload["roots"] = kRoots;
  payload["selected_root"] = kSelectedRoot.value_or("");
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["average_duration_seconds"] = 0;
  payload["total_duration_seconds"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  if (kWindow.explicit_range.has_value()) {
    payload["from_date"] = kWindow.explicit_range->start_date;
    payload["to_date"] = kWindow.explicit_range->end_date;
  }
  payload["series"] = json::array();

  const auto kAnyTrackedDate =
      infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kAnyTrackedDate.has_value()) {
    return payload.dump();
  }

  const auto& range = kWindow.range;
  if (!kWindow.explicit_range.has_value()) {
    payload["from_date"] = range.start_date;
    payload["to_date"] = range.end_date;
  }

  const std::vector<infra_data_query::DayDurationRow> kSparseRows =
      infra_data_query::QueryDayDurationsByRootInDateRange(
          db_conn, kSelectedRoot, range.start_date, range.end_date);
  const auto kSeriesResult = infra_data_query_stats::BuildInsightsChartSeries(
      {.start_date = range.start_date, .end_date = range.end_date}, kSparseRows,
      request.average_day_basis);
  for (const auto& point : kSeriesResult.series) {
    payload["series"].push_back(json{
        {"date", point.date},
        {"duration_seconds", point.duration_seconds},
        {"epoch_day", point.epoch_day},
    });
  }
  payload["average_duration_seconds"] =
      kSeriesResult.stats.average_duration_seconds;
  payload["total_duration_seconds"] =
      kSeriesResult.stats.total_duration_seconds;
  payload["active_days"] = kSeriesResult.stats.active_days;
  payload["range_days"] = kSeriesResult.stats.range_days;
  payload["average_denominator_days"] =
      kSeriesResult.stats.average_denominator_days;

  return payload.dump();
}

auto BuildInsightsCompositionContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  ValidateInsightsCompositionRequest(request);
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
  const auto kWindow = ResolveInsightsQueryWindow(
      request, kDefaultInsightsCompositionLookbackDays, kRangeErrors);

  json payload = json::object();
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["display_level"] = 0;
  payload["display_path"] = json::array();
  payload["total_duration_seconds"] = 0;
  payload["active_root_count"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  payload["average_day_basis"] = AverageDayBasisName(request.average_day_basis);
  payload["average_denominator_days"] = 0;
  const int kInitialRangeDays =
      infra_data_query_stats::CalculateInclusiveDateRangeDays(
          kWindow.range.start_date, kWindow.range.end_date);
  payload["range_days"] = kInitialRangeDays;
  if (request.average_day_basis == InsightsAverageDayBasis::kCalendarDays) {
    payload["average_denominator_days"] = kInitialRangeDays;
  }
  payload["tree"] = json::array();
  if (kWindow.explicit_range.has_value()) {
    payload["from_date"] = kWindow.explicit_range->start_date;
    payload["to_date"] = kWindow.explicit_range->end_date;
  }

  const auto kAnyTrackedDate =
      infra_data_query::QueryLatestTrackedDate(db_conn);
  if (!kAnyTrackedDate.has_value()) {
    return payload.dump();
  }

  const auto& range = kWindow.range;
  if (!kWindow.explicit_range.has_value()) {
    payload["from_date"] = range.start_date;
    payload["to_date"] = range.end_date;
  }

  infra_data_query::QueryFilters filters;
  filters.from_date = range.start_date;
  filters.to_date = range.end_date;
  const insights::ProjectTree kTree =
      infra_data_query::QueryProjectTree(db_conn, filters);
  const auto kCompositionStats =
      infra_data_query_stats::BuildInsightsCompositionStats(
          kTree, infra_data_query::QueryDayDurations(db_conn, filters),
          request.average_day_basis,
          infra_data_query_stats::CalculateInclusiveDateRangeDays(
              range.start_date, range.end_date));
  const int kRangeDays =
      infra_data_query_stats::CalculateInclusiveDateRangeDays(range.start_date,
                                                              range.end_date);
  const std::int64_t kTotalDurationSeconds = std::accumulate(
      kTree.begin(), kTree.end(), static_cast<std::int64_t>(0),
      [](std::int64_t total, const auto& entry) {
        const auto& [root_name, node] = entry;
        return total +
               (!root_name.empty() && node.duration > 0 ? node.duration : 0);
      });
  const int kActiveRootCount = static_cast<int>(
      std::count_if(kTree.begin(), kTree.end(), [](const auto& entry) {
        const auto& [root_name, node] = entry;
        return !root_name.empty() && node.duration > 0;
      }));
  if (kActiveRootCount == 1) {
    for (const auto& [root_name, node] : kTree) {
      if (!root_name.empty() && node.duration > 0 && !node.children.empty()) {
        payload["display_level"] = 1;
        payload["display_path"] = json::array({root_name});
        break;
      }
    }
  }
  payload["total_duration_seconds"] = kTotalDurationSeconds;
  payload["active_root_count"] = kActiveRootCount;
  payload["active_days"] = kCompositionStats.active_days;
  payload["range_days"] = kRangeDays;
  payload["average_denominator_days"] =
      request.average_day_basis == InsightsAverageDayBasis::kCalendarDays
          ? kRangeDays
          : kCompositionStats.active_days;
  payload["tree"] = BuildCompositionTreePayload(
      infra_data_query_stats::BuildInsightsCompositionTreeView(kTree),
      kCompositionStats);

  return payload.dump();
}

}  // namespace tracer::core::infrastructure::query::data::internal
