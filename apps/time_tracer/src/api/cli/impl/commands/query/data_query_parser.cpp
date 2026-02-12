// api/cli/impl/commands/query/data_query_parser.cpp
#include "api/cli/impl/commands/query/data_query_parser.hpp"

#include <stdexcept>
#include <string>

#include "api/cli/impl/utils/date_utils.hpp"

using namespace time_tracer::cli::impl::utils;

namespace time_tracer::cli::impl::commands::query::data {

auto ParseQueryFilters(const ParsedArgs& args) -> QueryFilters {
  QueryFilters filters;
  if (args.Has("year")) {
    filters.year = args.GetAsInt("year");
  }
  if (args.Has("month")) {
    filters.month = args.GetAsInt("month");
  }
  if (args.Has("from")) {
    filters.from_date = NormalizeDateInput(args.Get("from"), false);
  }
  if (args.Has("to")) {
    filters.to_date = NormalizeDateInput(args.Get("to"), true);
  }
  if (args.Has("remark")) {
    filters.remark = args.Get("remark");
  }
  if (args.Has("day_remark")) {
    filters.day_remark = args.Get("day_remark");
  }
  if (args.Has("project")) {
    filters.project = args.Get("project");
  }
  if (args.Has("exercise")) {
    filters.exercise = args.GetAsInt("exercise");
  }
  if (args.Has("status")) {
    filters.status = args.GetAsInt("status");
  }
  filters.overnight = args.Has("overnight");
  filters.reverse = args.Has("reverse");
  if (args.Has("numbers")) {
    filters.limit = args.GetAsInt("numbers");
  }
  return filters;
}

auto ResolveDataQueryAction(const ParsedArgs& args) -> DataQueryAction {
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

}  // namespace time_tracer::cli::impl::commands::query::data
