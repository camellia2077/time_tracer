#include <nlohmann/json.hpp>

#include <string>
#include <vector>

#include "infra/query/data/internal/insights_mapping.hpp"
#include "infra/query/data/repository/query_runtime_service_insights_content_support.hpp"

import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

namespace infra_data_query = tracer::core::infrastructure::query::data;
namespace infra_data_query_orchestrators =
    tracer::core::infrastructure::query::data::orchestrators;
namespace infra_data_query_stats =
    tracer::core::infrastructure::query::data::stats;

namespace tracer::core::infrastructure::query::data::internal {
namespace {

constexpr int kDefaultInsightsChartLookbackDays = 7;

using nlohmann::json;

auto BuildInsightsChartRangeErrors()
    -> infra_data_query_orchestrators::ExplicitDateRangeErrors {
  return {
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
}

}  // namespace

auto BuildInsightsChartContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  const auto kSelectedRoot = ResolveRequestedRootFilter(request);
  const std::vector<std::string> kRoots =
      infra_data_query::QueryProjectRootNames(db_conn);

  ValidateInsightsChartRequest(request);
  const auto kWindow =
      ResolveInsightsQueryWindow(request, kDefaultInsightsChartLookbackDays,
                                 BuildInsightsChartRangeErrors());

  json payload = json::object();
  payload["roots"] = kRoots;
  payload["selected_root"] = kSelectedRoot.value_or("");
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["average_duration_seconds"] = 0;
  payload["total_occurrence_count"] = 0;
  payload["average_duration_per_occurrence_seconds"] = 0;
  payload["mode_duration_seconds"] = nullptr;
  payload["median_duration_seconds"] = 0.0;
  payload["minimum_duration_seconds"] = 0.0;
  payload["maximum_duration_seconds"] = 0.0;
  payload["lower_quartile_duration_seconds"] = 0.0;
  payload["upper_quartile_duration_seconds"] = 0.0;
  payload["coefficient_of_variation"] = 0.0;
  payload["mean_absolute_deviation_seconds"] = 0.0;
  payload["total_duration_seconds"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  if (kWindow.explicit_range.has_value()) {
    payload["from_date"] = kWindow.explicit_range->start_date;
    payload["to_date"] = kWindow.explicit_range->end_date;
  }
  payload["series"] = json::array();
  payload["root_tree"] = json::array();

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

  // The picker needs the complete hierarchy so a previously persisted scope
  // remains navigable even when it has no records in the current date range.
  infra_data_query::QueryFilters root_tree_filters;
  payload["root_tree"] = BuildInsightsChartRootTreePayload(
      infra_data_query::QueryProjectTree(db_conn, root_tree_filters));

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
  payload["total_occurrence_count"] =
      kSeriesResult.stats.activity.occurrence_count;
  payload["average_duration_per_occurrence_seconds"] =
      kSeriesResult.stats.average_duration_per_occurrence_seconds;
  payload["mode_duration_seconds"] =
      kSeriesResult.stats.mode_duration_seconds.has_value()
          ? nlohmann::json(*kSeriesResult.stats.mode_duration_seconds)
          : nlohmann::json(nullptr);
  payload["median_duration_seconds"] =
      kSeriesResult.stats.median_duration_seconds;
  payload["minimum_duration_seconds"] =
      kSeriesResult.stats.minimum_duration_seconds;
  payload["maximum_duration_seconds"] =
      kSeriesResult.stats.maximum_duration_seconds;
  payload["lower_quartile_duration_seconds"] =
      kSeriesResult.stats.lower_quartile_duration_seconds;
  payload["upper_quartile_duration_seconds"] =
      kSeriesResult.stats.upper_quartile_duration_seconds;
  payload["coefficient_of_variation"] =
      kSeriesResult.stats.coefficient_of_variation;
  payload["mean_absolute_deviation_seconds"] =
      kSeriesResult.stats.mean_absolute_deviation_seconds;
  payload["total_duration_seconds"] =
      kSeriesResult.stats.activity.total_duration_seconds;
  payload["active_days"] = kSeriesResult.stats.active_days;
  payload["range_days"] = kSeriesResult.stats.range_days;
  payload["average_denominator_days"] =
      kSeriesResult.stats.average_denominator_days;

  return payload.dump();
}

}  // namespace tracer::core::infrastructure::query::data::internal
