inline constexpr int kDefaultActivityFrequentLookbackDays = 10;
inline constexpr int kDefaultActivityFrequentLimit = 5;

enum class DataQueryAction {
  kYears,
  kMonths,
  kDays,
  kDaysDuration,
  kDaysStats,
  kSearch,
  kActivityFrequent,
  kInsightsChart,
  kInsightsComposition,
  kTree
};

inline constexpr std::string_view kSupportedDataQueryActions =
    "years, months, days, days-duration, days-stats, search, "
    "activity-frequent, insights-chart, insights-composition, tree";

struct DayDurationRow {
  std::string date;
  long long total_seconds = 0;
  long long record_count = 0;
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

struct ActivityFrequentQueryOptions {
  int lookback_days = kDefaultActivityFrequentLookbackDays;
  int limit = kDefaultActivityFrequentLimit;
  std::optional<std::string> anchor_date;
  std::optional<std::string> prefix;
  bool score_by_duration = false;
};

struct ActivityFrequentRow {
  std::string activity_name;
  int usage_count = 0;
  long long total_duration_seconds = 0;
  std::string last_used_date;
  double score = 0.0;
};
