// api/cli/impl/commands/query/data_query_parser.cpp
#include "api/cli/impl/commands/query/data_query_parser.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/impl/utils/tree_formatter.hpp"

using namespace time_tracer::cli::impl::utils;
using time_tracer::core::dto::DataQueryAction;
using time_tracer::core::dto::DataQueryRequest;

namespace time_tracer::cli::impl::commands::query::data {

namespace {

constexpr std::string_view kSupportedDataQueryActions =
    "years, months, days, days-duration, days-stats, search, activity-suggest, "
    "mapping-names, report-chart, tree";
constexpr std::string_view kSupportedDataOutputModes = "text, json";
constexpr std::string_view kSupportedScoreModes = "frequency, duration";
constexpr std::string_view kSupportedPeriods =
    "day, week, month, year, recent, range";

[[nodiscard]] auto ParseDataQueryAction(const ParsedArgs &args)
    -> DataQueryAction {
  std::string action = "unknown";
  if (args.Has("action")) {
    action = args.Get("action");
  } else if (args.Has("argument")) {
    action = args.Get("argument");
  }

  if (action == "years") {
    return DataQueryAction::kYears;
  }
  if (action == "months") {
    return DataQueryAction::kMonths;
  }
  if (action == "days") {
    return DataQueryAction::kDays;
  }
  if (action == "days-duration") {
    return DataQueryAction::kDaysDuration;
  }
  if (action == "days-stats") {
    return DataQueryAction::kDaysStats;
  }
  if (action == "search") {
    return DataQueryAction::kSearch;
  }
  if (action == "activity-suggest") {
    return DataQueryAction::kActivitySuggest;
  }
  if (action == "mapping-names") {
    return DataQueryAction::kMappingNames;
  }
  if (action == "report-chart") {
    return DataQueryAction::kReportChart;
  }
  if (action == "tree") {
    return DataQueryAction::kTree;
  }

  throw std::runtime_error("Invalid query data action: " + action + ". Use " +
                           std::string(kSupportedDataQueryActions) + ".");
}

[[nodiscard]] auto ParseActivityScoreMode(std::string_view mode) -> bool {
  if (mode.empty() || mode == "frequency") {
    return false;
  }
  if (mode == "duration") {
    return true;
  }
  throw std::runtime_error("Invalid score mode: " + std::string(mode) +
                           ". Use " + std::string(kSupportedScoreModes) + ".");
}

[[nodiscard]] auto ParseDataOutputMode(std::string_view mode)
    -> time_tracer::core::dto::DataQueryOutputMode {
  using time_tracer::core::dto::DataQueryOutputMode;
  if (mode.empty() || mode == "text") {
    return DataQueryOutputMode::kText;
  }
  if (mode == "json" || mode == "semantic_json" || mode == "semantic-json") {
    return DataQueryOutputMode::kSemanticJson;
  }
  throw std::runtime_error("Invalid --data-output value: " + std::string(mode) +
                           ". Use " + std::string(kSupportedDataOutputModes) +
                           ".");
}

[[nodiscard]] auto IsSupportedPeriod(std::string_view value) -> bool {
  return value == "day" || value == "week" || value == "month" ||
         value == "year" || value == "recent" || value == "range";
}

} // namespace

auto ParseDataQueryRequest(const ParsedArgs &args) -> DataQueryRequest {
  DataQueryRequest request;
  request.action = ParseDataQueryAction(args);
  if (args.Has("data_output")) {
    request.output_mode = ParseDataOutputMode(args.Get("data_output"));
  }

  if (args.Has("year")) {
    request.year = args.GetAsInt("year");
  }
  if (args.Has("month")) {
    request.month = args.GetAsInt("month");
  }
  if (args.Has("from")) {
    request.from_date = NormalizeDateInput(args.Get("from"), false);
  }
  if (args.Has("to")) {
    request.to_date = NormalizeDateInput(args.Get("to"), true);
  }
  if (args.Has("remark")) {
    request.remark = args.Get("remark");
  }
  if (args.Has("day_remark")) {
    request.day_remark = args.Get("day_remark");
  }
  if (args.Has("project")) {
    request.project = args.Get("project");
  }
  if (args.Has("root")) {
    request.root = args.Get("root");
  }
  if (args.Has("exercise")) {
    request.exercise = args.GetAsInt("exercise");
  }
  if (args.Has("status")) {
    request.status = args.GetAsInt("status");
  }
  request.overnight = args.Has("overnight");
  request.reverse = args.Has("reverse");
  if (args.Has("numbers")) {
    request.limit = args.GetAsInt("numbers");
  }
  if (args.Has("top")) {
    request.top_n = args.GetAsInt("top");
  }
  if (args.Has("lookback_days")) {
    const int lookback_days = args.GetAsInt("lookback_days");
    if (lookback_days <= 0) {
      throw std::runtime_error("--lookback-days must be greater than 0.");
    }
    request.lookback_days = lookback_days;
  }
  if (args.Has("activity_prefix")) {
    request.activity_prefix = args.Get("activity_prefix");
  }
  if (args.Has("score_mode")) {
    request.activity_score_by_duration =
        ParseActivityScoreMode(args.Get("score_mode"));
  }
  if (args.Has("period")) {
    request.tree_period = args.Get("period");
  }
  if (args.Has("period_arg")) {
    request.tree_period_argument = args.Get("period_arg");
  }
  if (args.Has("level")) {
    request.tree_max_depth = args.GetAsInt("level");
  }

  const bool supports_period = request.action == DataQueryAction::kTree ||
                               request.action == DataQueryAction::kDaysStats;

  if (supports_period) {
    if (request.tree_period.has_value()) {
      const std::string period = *request.tree_period;
      if (!IsSupportedPeriod(period)) {
        throw std::runtime_error("Invalid --period value: " + period +
                                 ". Use " + std::string(kSupportedPeriods) +
                                 ".");
      }
      if (!request.tree_period_argument.has_value() && period != "recent") {
        if (request.action == DataQueryAction::kTree) {
          throw std::runtime_error(
              "query data tree with --period requires --period-arg "
              "(except recent, where --lookback-days can be used).");
        }
        throw std::runtime_error(
            "query data days-stats with --period requires --period-arg "
            "(except recent, where --lookback-days can be used).");
      }
    }
  }

  if (request.action == DataQueryAction::kTree &&
      !request.tree_period.has_value() && !args.Has("from") &&
      !args.Has("to") && !args.Has("year") && !args.Has("month") &&
      !args.Has("lookback_days")) {
    throw std::runtime_error(
        "query data tree requires either --period/--period-arg or "
        "date filters (--from/--to/--year/--month).");
  }

  return request;
}

} // namespace time_tracer::cli::impl::commands::query::data
