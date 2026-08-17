// infra/query/data/stats/stats_models.hpp
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "domain/insights/models/activity_aggregate.hpp"

namespace tracer::core::infrastructure::query::data::stats {

struct InsightsChartSeriesPoint {
  std::string date;
  long long duration_seconds = 0;
  long long epoch_day = 0;
};

struct InsightsChartAggregateStats {
  ActivityAggregate activity;
  long long average_duration_seconds = 0;
  long long average_duration_per_occurrence_seconds = 0;
  std::optional<double> mode_duration_seconds;
  double median_duration_seconds = 0.0;
  double minimum_duration_seconds = 0.0;
  double maximum_duration_seconds = 0.0;
  double lower_quartile_duration_seconds = 0.0;
  double upper_quartile_duration_seconds = 0.0;
  double coefficient_of_variation = 0.0;
  double mean_absolute_deviation_seconds = 0.0;
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
using tracer::core::infrastructure::query::data::stats::
    InsightsChartSeriesPoint;
using tracer::core::infrastructure::query::data::stats::
    InsightsChartSeriesResult;

}  // namespace tracer_core::infrastructure::query::data::stats
