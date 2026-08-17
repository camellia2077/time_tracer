#include <nlohmann/json.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>

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

constexpr int kDefaultInsightsCompositionLookbackDays = 7;

using nlohmann::json;

auto BuildInsightsCompositionRangeErrors()
    -> infra_data_query_orchestrators::ExplicitDateRangeErrors {
  return {
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
}

}  // namespace

auto BuildInsightsCompositionContent(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request)
    -> std::string {
  ValidateInsightsCompositionRequest(request);
  const auto kWindow = ResolveInsightsQueryWindow(
      request, kDefaultInsightsCompositionLookbackDays,
      BuildInsightsCompositionRangeErrors());

  json payload = json::object();
  payload["lookback_days"] = kWindow.payload_lookback_days;
  payload["display_level"] = 0;
  payload["display_path"] = json::array();
  payload["total_duration_seconds"] = 0;
  payload["active_root_count"] = 0;
  payload["active_days"] = 0;
  payload["range_days"] = 0;
  payload["average_day_basis"] = AverageDayBasisName(request.average_day_basis);
  const int kRangeDays =
      infra_data_query_stats::CalculateInclusiveDateRangeDays(
          kWindow.range.start_date, kWindow.range.end_date);
  payload["range_days"] = kRangeDays;
  payload["average_denominator_days"] =
      infra_data_query_stats::ResolveAverageDenominator(
          request.average_day_basis, 0, kRangeDays);
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
          request.average_day_basis, kRangeDays);
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
      kCompositionStats.average_denominator_days;
  payload["tree"] = BuildCompositionTreePayload(
      infra_data_query_stats::BuildInsightsCompositionTreeView(kTree),
      kCompositionStats);

  return payload.dump();
}

}  // namespace tracer::core::infrastructure::query::data::internal
