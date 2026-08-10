// infra/query/data/stats/insights_chart_stats_calculator.hpp
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain/insights/models/project_tree.hpp"
#include "application/dto/query_requests.hpp"
#include "infra/query/data/data_query_types.hpp"
#include "infra/query/data/stats/stats_models.hpp"

namespace tracer::core::infrastructure::query::data::stats {

struct InsightsChartDateRange {
  std::string_view start_date;
  std::string_view end_date;
};

[[nodiscard]] auto CalculateInclusiveDateRangeDays(std::string_view start_date,
                                                   std::string_view end_date)
    -> int;

template <typename TValue>
[[nodiscard]] constexpr auto AverageOrZero(TValue total, int denominator)
    -> TValue {
  return denominator > 0 ? total / static_cast<TValue>(denominator) : TValue{};
}

struct InsightsCompositionTreeNodeView {
  std::string name;
  std::string path;
  const insights::ProjectNode* node = nullptr;
  std::int64_t level_occurrence_count = 0;
  std::vector<InsightsCompositionTreeNodeView> children;
};

[[nodiscard]] auto BuildInsightsCompositionTreeView(
    const insights::ProjectTree& tree)
    -> std::vector<InsightsCompositionTreeNodeView>;

[[nodiscard]] auto BuildInsightsChartSeries(
    InsightsChartDateRange range, const std::vector<DayDurationRow>& sparse_rows,
    tracer_core::core::dto::InsightsAverageDayBasis average_day_basis =
        tracer_core::core::dto::InsightsAverageDayBasis::kActiveDays)
    -> InsightsChartSeriesResult;

struct InsightsCompositionNodeStats {
  std::int64_t average_duration_seconds = 0;
  double average_occurrence_count = 0.0;
  double average_occurrence_ratio = 0.0;
};

struct InsightsCompositionStats {
  int active_days = 0;
  std::unordered_map<std::string, InsightsCompositionNodeStats> nodes;
};

[[nodiscard]] auto BuildInsightsCompositionStats(
    const insights::ProjectTree& tree,
    const std::vector<DayDurationRow>& recorded_days,
    tracer_core::core::dto::InsightsAverageDayBasis average_day_basis =
        tracer_core::core::dto::InsightsAverageDayBasis::kActiveDays,
    int range_days = 0) -> InsightsCompositionStats;

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::BuildInsightsChartSeries;

}  // namespace tracer_core::infrastructure::query::data::stats
