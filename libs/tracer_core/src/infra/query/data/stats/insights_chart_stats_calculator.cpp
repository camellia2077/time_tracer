// infra/query/data/stats/insights_chart_stats_calculator.cpp
#include "infra/query/data/stats/insights_chart_stats_calculator.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

import tracer.core.infrastructure.query.data.repository.types;
import tracer.core.infrastructure.query.data.stats.models;

namespace tracer::core::infrastructure::query::data::stats {
namespace {

constexpr size_t kIsoDateLength = 10;
constexpr size_t kIsoDateYearSeparatorIndex = 4;
constexpr size_t kIsoDateMonthSeparatorIndex = 7;
constexpr size_t kIsoDateYearLength = 4;
constexpr size_t kIsoDateMonthOffset = 5;
constexpr size_t kIsoDateDayOffset = 8;
constexpr size_t kIsoDateMonthDayLength = 2;
constexpr int kDecimalBase = 10;

using LegacyDayDurationRow =
    ::tracer_core::infrastructure::query::data::DayDurationRow;

auto ParseUnsigned(std::string_view value, int& out) -> bool {
  if (value.empty()) {
    return false;
  }
  int parsed = 0;
  for (const char kCharacter : value) {
    if (kCharacter < '0' || kCharacter > '9') {
      return false;
    }
    parsed = (parsed * kDecimalBase) + (kCharacter - '0');
  }
  out = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value)
    -> std::optional<std::chrono::year_month_day> {
  if (value.size() != kIsoDateLength ||
      value[kIsoDateYearSeparatorIndex] != '-' ||
      value[kIsoDateMonthSeparatorIndex] != '-') {
    return std::nullopt;
  }
  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0, kIsoDateYearLength), year) ||
      !ParseUnsigned(value.substr(kIsoDateMonthOffset, kIsoDateMonthDayLength),
                     month) ||
      !ParseUnsigned(value.substr(kIsoDateDayOffset, kIsoDateMonthDayLength),
                     day)) {
    return std::nullopt;
  }
  const std::chrono::year_month_day kYmd{
      std::chrono::year{year}, std::chrono::month{static_cast<unsigned>(month)},
      std::chrono::day{static_cast<unsigned>(day)}};
  if (!kYmd.ok()) {
    return std::nullopt;
  }
  return kYmd;
}

auto FormatIsoDate(const std::chrono::year_month_day& ymd) -> std::string {
  std::ostringstream stream;
  stream << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << "-"
         << std::setw(2) << std::setfill('0') << unsigned(ymd.day());
  return stream.str();
}

struct DaySummary {
  long long total_seconds = 0;
  long long record_count = 0;
};

struct ChartDistributionStats {
  std::optional<double> mode_seconds;
  double median_seconds = 0.0;
  double minimum_seconds = 0.0;
  double maximum_seconds = 0.0;
  double standard_deviation_seconds = 0.0;
  double lower_quartile_seconds = 0.0;
  double upper_quartile_seconds = 0.0;
  double coefficient_of_variation = 0.0;
  double mean_absolute_deviation_seconds = 0.0;
};

auto InterpolatedPercentile(const std::vector<long long>& sorted_durations,
                            double percentile) -> double {
  if (sorted_durations.empty()) {
    return 0.0;
  }
  const double position =
      static_cast<double>(sorted_durations.size() - 1) * percentile;
  const size_t lower_index = static_cast<size_t>(position);
  const size_t upper_index =
      std::min(lower_index + 1, sorted_durations.size() - 1);
  const double fraction = position - static_cast<double>(lower_index);
  const double lower = static_cast<double>(sorted_durations[lower_index]);
  const double upper = static_cast<double>(sorted_durations[upper_index]);
  return lower + ((upper - lower) * fraction);
}

auto CalculateChartDistributionStats(const std::vector<long long>& durations)
    -> ChartDistributionStats {
  if (durations.empty()) {
    return {};
  }

  std::unordered_map<long long, int> frequencies;
  frequencies.reserve(durations.size());
  for (const long long duration_seconds : durations) {
    ++frequencies[duration_seconds];
  }

  std::vector<long long> sorted_durations = durations;
  std::ranges::sort(sorted_durations);
  const size_t kMiddle = sorted_durations.size() / 2;
  const double kMedian =
      sorted_durations.size() % 2 == 1
          ? static_cast<double>(sorted_durations[kMiddle])
          : (static_cast<double>(sorted_durations[kMiddle - 1]) +
             static_cast<double>(sorted_durations[kMiddle])) /
                2.0;

  int mode_frequency = 0;
  std::optional<long long> mode_seconds;
  for (const auto& [duration_seconds, frequency] : frequencies) {
    if (frequency < 2 ||
        (mode_seconds.has_value() && frequency < mode_frequency)) {
      continue;
    }
    if (!mode_seconds.has_value() || frequency > mode_frequency ||
        (frequency == mode_frequency && duration_seconds < *mode_seconds)) {
      mode_frequency = frequency;
      mode_seconds = duration_seconds;
    }
  }

  double mean = 0.0;
  double sum_squared_delta = 0.0;
  int sample_count = 0;
  for (const long long duration_seconds : durations) {
    ++sample_count;
    const double kValue = static_cast<double>(duration_seconds);
    const double kDelta = kValue - mean;
    mean += kDelta / static_cast<double>(sample_count);
    const double kDelta2 = kValue - mean;
    sum_squared_delta += kDelta * kDelta2;
  }
  const double standard_deviation =
      std::sqrt(sum_squared_delta / static_cast<double>(sample_count));
  double absolute_delta_sum = 0.0;
  for (const long long duration_seconds : durations) {
    absolute_delta_sum +=
        std::abs(static_cast<double>(duration_seconds) - mean);
  }

  return {
      .mode_seconds =
          mode_seconds.has_value()
              ? std::optional<double>{static_cast<double>(*mode_seconds)}
              : std::nullopt,
      .median_seconds = kMedian,
      .minimum_seconds = static_cast<double>(sorted_durations.front()),
      .maximum_seconds = static_cast<double>(sorted_durations.back()),
      .standard_deviation_seconds = standard_deviation,
      .lower_quartile_seconds = InterpolatedPercentile(sorted_durations, 0.25),
      .upper_quartile_seconds = InterpolatedPercentile(sorted_durations, 0.75),
      .coefficient_of_variation = mean > 0.0 ? standard_deviation / mean : 0.0,
      .mean_absolute_deviation_seconds =
          absolute_delta_sum / static_cast<double>(sample_count),
  };
}

auto BuildTotalsByDate(const std::vector<LegacyDayDurationRow>& sparse_rows)
    -> std::unordered_map<std::string, DaySummary> {
  std::unordered_map<std::string, DaySummary> totals_by_date;
  totals_by_date.reserve(sparse_rows.size());
  for (const auto& row : sparse_rows) {
    totals_by_date[row.date] = DaySummary{.total_seconds = row.total_seconds,
                                          .record_count = row.record_count};
  }
  return totals_by_date;
}

auto BuildCompositionTreeNodeView(const std::string& name,
                                  const insights::ProjectNode& node,
                                  std::string path,
                                  std::int64_t level_occurrence_count)
    -> InsightsCompositionTreeNodeView {
  InsightsCompositionTreeNodeView view{
      .name = name,
      .path = std::move(path),
      .node = &node,
      .level_occurrence_count = level_occurrence_count,
  };

  std::vector<std::pair<std::string_view, const insights::ProjectNode*>>
      children;
  for (const auto& [child_name, child] : node.children) {
    if (!child_name.empty() && child.occurrence_count > 0) {
      children.push_back({child_name, &child});
    }
  }
  std::ranges::sort(children, [](const auto& left, const auto& right) {
    return left.first < right.first;
  });

  std::int64_t child_occurrence_count = 0;
  for (const auto& [child_name, child] : children) {
    static_cast<void>(child_name);
    child_occurrence_count += child->occurrence_count;
  }
  view.children.reserve(children.size());
  for (const auto& [child_name, child] : children) {
    view.children.push_back(BuildCompositionTreeNodeView(
        std::string(child_name), *child,
        view.path + "_" + std::string(child_name), child_occurrence_count));
  }
  return view;
}

}  // namespace

auto CalculateInclusiveDateRangeDays(std::string_view start_date,
                                     std::string_view end_date) -> int {
  const auto kStartYmd = ParseIsoDate(start_date);
  const auto kEndYmd = ParseIsoDate(end_date);
  if (!kStartYmd.has_value() || !kEndYmd.has_value()) {
    throw std::runtime_error("invalid ISO date range.");
  }

  const auto kStartDays = std::chrono::sys_days{*kStartYmd};
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  if (kStartDays > kEndDays) {
    throw std::runtime_error(
        "date range start must be before or equal to end.");
  }
  return static_cast<int>((kEndDays - kStartDays).count()) + 1;
}

auto BuildInsightsCompositionTreeView(const insights::ProjectTree& tree)
    -> std::vector<InsightsCompositionTreeNodeView> {
  std::vector<std::pair<std::string_view, const insights::ProjectNode*>> roots;
  for (const auto& [root_name, root] : tree) {
    if (!root_name.empty() && root.occurrence_count > 0) {
      roots.push_back({root_name, &root});
    }
  }
  std::ranges::sort(roots, [](const auto& left, const auto& right) {
    return left.first < right.first;
  });

  std::int64_t root_occurrence_count = 0;
  for (const auto& [root_name, root] : roots) {
    static_cast<void>(root_name);
    root_occurrence_count += root->occurrence_count;
  }

  std::vector<InsightsCompositionTreeNodeView> views;
  views.reserve(roots.size());
  for (const auto& [root_name, root] : roots) {
    views.push_back(BuildCompositionTreeNodeView(std::string(root_name), *root,
                                                 std::string(root_name),
                                                 root_occurrence_count));
  }
  return views;
}

auto AppendCompositionNodeStats(
    const std::vector<InsightsCompositionTreeNodeView>& views,
    int denominator_days, InsightsCompositionStats& result) -> void {
  for (const auto& view : views) {
    const ActivityAggregate kActivityAggregate{
        .total_duration_seconds = view.node->duration,
        .occurrence_count = view.node->occurrence_count};
    result.nodes.emplace(
        view.path,
        InsightsCompositionNodeStats{
            .average_duration_seconds =
                CalculateAverageOrZero(view.node->duration, denominator_days),
            .average_duration_per_occurrence_seconds =
                kActivityAggregate.AverageDurationPerOccurrenceSeconds(),
            .average_occurrence_count = CalculateAverageOrZero(
                static_cast<double>(view.node->occurrence_count),
                denominator_days),
            .average_occurrence_ratio = CalculateAverageOrZero(
                static_cast<double>(view.node->occurrence_count),
                static_cast<int>(view.level_occurrence_count)),
        });
    AppendCompositionNodeStats(view.children, denominator_days, result);
  }
}

auto BuildInsightsChartSeries(
    InsightsChartDateRange range,
    const std::vector<LegacyDayDurationRow>& sparse_rows,
    tracer_core::core::dto::InsightsAverageDayBasis average_day_basis)
    -> InsightsChartSeriesResult {
  const auto kStartYmd = ParseIsoDate(range.start_date);
  const auto kEndYmd = ParseIsoDate(range.end_date);
  if (!kStartYmd.has_value() || !kEndYmd.has_value()) {
    throw std::runtime_error("insights-chart resolved invalid date range.");
  }

  const auto kStartDays = std::chrono::sys_days{*kStartYmd};
  const auto kEndDays = std::chrono::sys_days{*kEndYmd};
  const int kRangeDays =
      CalculateInclusiveDateRangeDays(range.start_date, range.end_date);

  const auto kTotalsByDate = BuildTotalsByDate(sparse_rows);

  InsightsChartSeriesResult result;
  result.series.reserve(static_cast<size_t>(kRangeDays));
  std::vector<long long> active_durations;
  active_durations.reserve(static_cast<size_t>(kRangeDays));
  for (auto cursor = kStartDays; cursor <= kEndDays;
       cursor += std::chrono::days{1}) {
    const std::string kDate =
        FormatIsoDate(std::chrono::year_month_day{cursor});
    const auto kIt = kTotalsByDate.find(kDate);
    const long long kDurationSeconds =
        kIt == kTotalsByDate.end() ? 0LL : kIt->second.total_seconds;
    const long long kEpochDay =
        static_cast<long long>(cursor.time_since_epoch().count());
    result.stats.activity.Add(kDurationSeconds, kIt == kTotalsByDate.end()
                                                    ? 0LL
                                                    : kIt->second.record_count);
    result.stats.range_days = kRangeDays;
    if (kIt != kTotalsByDate.end() &&
        (kIt->second.record_count > 0 || kDurationSeconds > 0)) {
      ++result.stats.active_days;
      active_durations.push_back(kDurationSeconds);
    }
    result.series.push_back(InsightsChartSeriesPoint{
        .date = kDate,
        .duration_seconds = kDurationSeconds,
        .epoch_day = kEpochDay,
    });
  }

  result.stats.average_denominator_days = ResolveAverageDenominator(
      average_day_basis, result.stats.active_days, result.stats.range_days);
  result.stats.average_duration_seconds =
      CalculateAverageOrZero(result.stats.activity.total_duration_seconds,
                             result.stats.average_denominator_days);
  result.stats.average_duration_per_occurrence_seconds =
      result.stats.activity.AverageDurationPerOccurrenceSeconds();

  const auto kDistributionStats =
      CalculateChartDistributionStats(active_durations);
  result.stats.mode_duration_seconds = kDistributionStats.mode_seconds;
  result.stats.median_duration_seconds = kDistributionStats.median_seconds;
  result.stats.minimum_duration_seconds = kDistributionStats.minimum_seconds;
  result.stats.maximum_duration_seconds = kDistributionStats.maximum_seconds;
  result.stats.lower_quartile_duration_seconds =
      kDistributionStats.lower_quartile_seconds;
  result.stats.upper_quartile_duration_seconds =
      kDistributionStats.upper_quartile_seconds;
  result.stats.coefficient_of_variation =
      kDistributionStats.coefficient_of_variation;
  result.stats.mean_absolute_deviation_seconds =
      kDistributionStats.mean_absolute_deviation_seconds;

  return result;
}

auto BuildInsightsCompositionStats(
    const insights::ProjectTree& tree,
    const std::vector<LegacyDayDurationRow>& recorded_days,
    tracer_core::core::dto::InsightsAverageDayBasis average_day_basis,
    int range_days) -> InsightsCompositionStats {
  InsightsCompositionStats result;
  result.active_days = static_cast<int>(recorded_days.size());
  result.average_denominator_days = ResolveAverageDenominator(
      average_day_basis, result.active_days, range_days);
  AppendCompositionNodeStats(BuildInsightsCompositionTreeView(tree),
                             result.average_denominator_days, result);
  return result;
}

}  // namespace tracer::core::infrastructure::query::data::stats
