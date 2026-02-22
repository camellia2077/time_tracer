#pragma once

#include <string>
#include <vector>

namespace time_tracer::infrastructure::query::data::stats {

struct ReportChartSeriesPoint {
  std::string date;
  long long duration_seconds = 0;
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

}  // namespace time_tracer::infrastructure::query::data::stats
