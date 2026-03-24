#ifndef APPLICATION_DTO_QUERY_REQUESTS_HPP_
#define APPLICATION_DTO_QUERY_REQUESTS_HPP_

#include <optional>
#include <string>

namespace tracer_core::core::dto {

enum class DataQueryAction {
  kYears,
  kMonths,
  kDays,
  kDaysDuration,
  kDaysStats,
  kSearch,
  kActivitySuggest,
  kMappingNames,
  kReportChart,
  kTree,
};

enum class DataQueryOutputMode {
  kText,
  kSemanticJson,
};

struct DataQueryRequest {
  DataQueryAction action = DataQueryAction::kYears;
  DataQueryOutputMode output_mode = DataQueryOutputMode::kText;
  std::optional<int> year;
  std::optional<int> month;
  std::optional<std::string> from_date;
  std::optional<std::string> to_date;
  std::optional<std::string> remark;
  std::optional<std::string> day_remark;
  std::optional<std::string> project;
  std::optional<int> exercise;
  std::optional<int> status;
  bool overnight = false;
  bool reverse = false;
  std::optional<int> limit;
  std::optional<int> top_n;
  std::optional<int> lookback_days;
  std::optional<std::string> activity_prefix;
  bool activity_score_by_duration = false;
  std::optional<std::string> tree_period;
  std::optional<std::string> tree_period_argument;
  std::optional<int> tree_max_depth;
  std::optional<std::string> root;
};

struct TreeQueryRequest {
  bool list_roots = false;
  std::string root_pattern;
  int max_depth = -1;
  std::optional<std::string> period;
  std::optional<std::string> period_argument;
  std::optional<std::string> root;
};

}  // namespace tracer_core::core::dto

#endif  // APPLICATION_DTO_QUERY_REQUESTS_HPP_
