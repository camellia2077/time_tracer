#include "infrastructure/query/data/stats/day_duration_stats_calculator.hpp"

#include <algorithm>
#include <cmath>
#include <ranges>
#include <vector>

namespace time_tracer::infrastructure::query::data::stats {
namespace {

constexpr double kPercentile25 = 25.0;
constexpr double kPercentile75 = 75.0;
constexpr double kPercentile90 = 90.0;
constexpr double kPercentile95 = 95.0;
constexpr double kEvenCountDivisor = 2.0;

}  // namespace

auto ComputeDayDurationStats(const std::vector<DayDurationRow>& rows)
    -> DayDurationStats {
  DayDurationStats stats;
  if (rows.empty()) {
    return stats;
  }

  std::vector<long long> durations;
  durations.reserve(rows.size());
  for (const auto& row : rows) {
    durations.push_back(row.total_seconds);
  }
  std::ranges::sort(durations);

  auto percentile = [&](double percentile_value) -> double {
    if (durations.empty()) {
      return 0.0;
    }
    if (percentile_value <= 0.0) {
      return static_cast<double>(durations.front());
    }
    if (percentile_value >= 100.0) {
      return static_cast<double>(durations.back());
    }
    double rank = std::ceil((percentile_value / 100.0) *
                            static_cast<double>(durations.size()));
    size_t index = static_cast<size_t>(std::max(1.0, rank)) - 1;
    if (index >= durations.size()) {
      index = durations.size() - 1;
    }
    return static_cast<double>(durations[index]);
  };

  double median = 0.0;
  if (durations.size() % 2 == 1) {
    median = static_cast<double>(durations[durations.size() / 2]);
  } else {
    size_t mid = durations.size() / 2;
    median = (static_cast<double>(durations[mid - 1]) +
              static_cast<double>(durations[mid])) /
             kEvenCountDivisor;
  }

  std::vector<double> deviations;
  deviations.reserve(durations.size());
  for (const auto& value : durations) {
    deviations.push_back(std::abs(static_cast<double>(value) - median));
  }
  std::ranges::sort(deviations);

  double mad = 0.0;
  if (deviations.size() % 2 == 1) {
    mad = deviations[deviations.size() / 2];
  } else {
    size_t mid = deviations.size() / 2;
    mad = (deviations[mid - 1] + deviations[mid]) / kEvenCountDivisor;
  }

  double mean = 0.0;
  double sum_squared_delta = 0.0;
  int sample_count = 0;
  for (const auto& value : durations) {
    ++sample_count;
    const auto kValueAsDouble = static_cast<double>(value);
    const double kDelta = kValueAsDouble - mean;
    mean += kDelta / static_cast<double>(sample_count);
    const double kDelta2 = kValueAsDouble - mean;
    sum_squared_delta += kDelta * kDelta2;
  }

  stats.count = sample_count;
  stats.mean_seconds = mean;
  stats.variance_seconds =
      (sample_count > 0)
          ? (sum_squared_delta / static_cast<double>(sample_count))
          : 0.0;
  stats.stddev_seconds = std::sqrt(stats.variance_seconds);
  stats.median_seconds = median;
  stats.p25_seconds = percentile(kPercentile25);
  stats.p75_seconds = percentile(kPercentile75);
  stats.p90_seconds = percentile(kPercentile90);
  stats.p95_seconds = percentile(kPercentile95);
  stats.min_seconds = static_cast<double>(durations.front());
  stats.max_seconds = static_cast<double>(durations.back());
  stats.iqr_seconds = stats.p75_seconds - stats.p25_seconds;
  stats.mad_seconds = mad;
  return stats;
}

}  // namespace time_tracer::infrastructure::query::data::stats
