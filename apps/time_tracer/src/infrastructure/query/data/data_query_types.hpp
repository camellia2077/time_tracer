// infrastructure/query/data/data_query_types.hpp
#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace time_tracer::infrastructure::query::data {

constexpr int kDefaultActivitySuggestionLookbackDays = 10;
constexpr int kDefaultActivitySuggestionLimit = 5;

enum class DataQueryAction {
  kYears,
  kMonths,
  kDays,
  kDaysDuration,
  kDaysStats,
  kSearch,
  kActivitySuggest,
  kReportChart,
  kTree
};

constexpr std::string_view kSupportedDataQueryActions =
    "years, months, days, days-duration, days-stats, search, "
    "activity-suggest, report-chart, tree";

struct DayDurationRow {
  std::string date;
  long long total_seconds = 0;
};

struct DayDurationStats {
  int count = 0;
  double mean_seconds = 0.0;
  double variance_seconds = 0.0;
  double stddev_seconds = 0.0;
  double median_seconds = 0.0;
  double p25_seconds = 0.0;
  double p75_seconds = 0.0;
  double p90_seconds = 0.0;
  double p95_seconds = 0.0;
  double min_seconds = 0.0;
  double max_seconds = 0.0;
  double iqr_seconds = 0.0;
  double mad_seconds = 0.0;
};

struct ActivitySuggestionQueryOptions {
  int lookback_days = kDefaultActivitySuggestionLookbackDays;
  int limit = kDefaultActivitySuggestionLimit;
  std::optional<std::string> prefix;
  bool score_by_duration = false;
};

struct ActivitySuggestionRow {
  std::string activity_name;
  int usage_count = 0;
  long long total_duration_seconds = 0;
  std::string last_used_date;
  double score = 0.0;
};

}  // namespace time_tracer::infrastructure::query::data
