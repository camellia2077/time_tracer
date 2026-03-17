// infrastructure/query/data/stats/stats_models.hpp
#pragma once

#include <string>
#include <vector>

namespace tracer::core::infrastructure::query::data::stats {

struct ReportChartSeriesPoint {
  std::string date;
  long long duration_seconds = 0;
  long long epoch_day = 0;
};

struct ReportChartAggregateStats {
  long long total_duration_seconds = 0;
  long long average_duration_seconds = 0;
  int active_days = 0;
  int range_days = 0;
};

struct ReportChartSeriesResult {
  std::vector<ReportChartSeriesPoint> series;
  ReportChartAggregateStats stats;
};

}  // namespace tracer::core::infrastructure::query::data::stats

namespace tracer_core::infrastructure::query::data::stats {

using tracer::core::infrastructure::query::data::stats::ReportChartAggregateStats;
using tracer::core::infrastructure::query::data::stats::ReportChartSeriesPoint;
using tracer::core::infrastructure::query::data::stats::ReportChartSeriesResult;

}  // namespace tracer_core::infrastructure::query::data::stats
