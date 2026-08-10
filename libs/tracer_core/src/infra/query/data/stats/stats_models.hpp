// infra/query/data/stats/stats_models.hpp
#pragma once

#include <string>
#include <vector>

namespace tracer::core::infrastructure::query::data::stats {

struct InsightsChartSeriesPoint {
  std::string date;
  long long duration_seconds = 0;
  long long epoch_day = 0;
};

struct InsightsChartAggregateStats {
  long long total_duration_seconds = 0;
  long long average_duration_seconds = 0;
  int active_days = 0;
  int range_days = 0;
  int average_denominator_days = 0;
};

struct InsightsChartSeriesResult {
  std::vector<InsightsChartSeriesPoint> series;
  InsightsChartAggregateStats stats;
};

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::
    InsightsChartAggregateStats;
using tracer::core::infrastructure::query::data::stats::InsightsChartSeriesPoint;
using tracer::core::infrastructure::query::data::stats::InsightsChartSeriesResult;

}  // namespace tracer_core::infrastructure::query::data::stats
