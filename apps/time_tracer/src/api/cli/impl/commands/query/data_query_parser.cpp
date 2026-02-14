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
    "years, months, days, days-duration, days-stats, search";

[[nodiscard]] auto ParseDataQueryAction(const ParsedArgs& args)
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

  throw std::runtime_error("Invalid query data action: " + action + ". Use " +
                           std::string(kSupportedDataQueryActions) + ".");
}

}  // namespace

auto ParseDataQueryRequest(const ParsedArgs& args) -> DataQueryRequest {
  DataQueryRequest request;
  request.action = ParseDataQueryAction(args);

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
  return request;
}

}  // namespace time_tracer::cli::impl::commands::query::data
