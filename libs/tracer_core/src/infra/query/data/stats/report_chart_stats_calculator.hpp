// infra/query/data/stats/report_chart_stats_calculator.hpp
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "domain/reports/models/project_tree.hpp"
#include "application/dto/query_requests.hpp"
#include "infra/query/data/data_query_types.hpp"
#include "infra/query/data/stats/stats_models.hpp"

namespace tracer::core::infrastructure::query::data::stats {

struct ReportChartDateRange {
  std::string_view start_date;
  std::string_view end_date;
};

[[nodiscard]] auto CalculateInclusiveDateRangeDays(
    std::string_view start_date, std::string_view end_date) -> int;

template <typename TValue>
[[nodiscard]] constexpr auto AverageOrZero(TValue total, int denominator)
    -> TValue {
  return denominator > 0 ? total / static_cast<TValue>(denominator) : TValue{};
}

struct ReportCompositionTreeNodeView {
  std::string name;
  std::string path;
  const reporting::ProjectNode* node = nullptr;
  std::int64_t level_occurrence_count = 0;
  std::vector<ReportCompositionTreeNodeView> children;
};

[[nodiscard]] auto BuildReportCompositionTreeView(
    const reporting::ProjectTree& tree)
    -> std::vector<ReportCompositionTreeNodeView>;

[[nodiscard]] auto BuildReportChartSeries(
    ReportChartDateRange range, const std::vector<DayDurationRow>& sparse_rows,
    tracer_core::core::dto::ReportAverageDayBasis average_day_basis =
        tracer_core::core::dto::ReportAverageDayBasis::kActiveDays)
    -> ReportChartSeriesResult;

struct ReportCompositionNodeStats {
  std::int64_t average_duration_seconds = 0;
  double average_occurrence_count = 0.0;
  double average_occurrence_ratio = 0.0;
};

struct ReportCompositionStats {
  int active_days = 0;
  std::unordered_map<std::string, ReportCompositionNodeStats> nodes;
};

[[nodiscard]] auto BuildReportCompositionStats(
    const reporting::ProjectTree& tree,
    const std::vector<DayDurationRow>& recorded_days,
    tracer_core::core::dto::ReportAverageDayBasis average_day_basis =
        tracer_core::core::dto::ReportAverageDayBasis::kActiveDays,
    int range_days = 0) -> ReportCompositionStats;

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::BuildReportChartSeries;

}  // namespace tracer_core::infrastructure::query::data::stats
