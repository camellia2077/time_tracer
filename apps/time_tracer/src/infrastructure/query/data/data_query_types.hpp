// infrastructure/query/data/data_query_types.hpp
#pragma once

#include <string>
#include <string_view>

namespace time_tracer::infrastructure::query::data {

enum class DataQueryAction {
  kYears,
  kMonths,
  kDays,
  kDaysDuration,
  kDaysStats,
  kSearch
};

constexpr std::string_view kSupportedDataQueryActions =
    "years, months, days, days-duration, days-stats, search";

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

}  // namespace time_tracer::infrastructure::query::data
