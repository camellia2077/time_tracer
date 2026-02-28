// infrastructure/query/data/orchestrators/list_query_orchestrator.cpp
#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"

#include <utility>

#include "infrastructure/query/data/data_query_repository.hpp"
#include "infrastructure/query/data/renderers/data_query_renderer.hpp"

namespace data_query_renderers =
    tracer_core::infrastructure::query::data::renderers;

namespace tracer_core::infrastructure::query::data::orchestrators {
namespace {

constexpr int kDefaultSuggestLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleYearsQuery(sqlite3* db_conn, bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  const auto kYears = QueryYears(db_conn);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("years", kYears, semantic_json));
}

auto HandleMonthsQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  const auto kMonths = QueryMonths(db_conn, base_filters.kYear);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("months", kMonths, semantic_json));
}

auto HandleDaysQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                     bool semantic_json) -> tracer_core::core::dto::TextOutput {
  const auto kDays = QueryDays(db_conn, base_filters.kYear, base_filters.kMonth,
                               base_filters.from_date, base_filters.to_date,
                               base_filters.reverse, base_filters.limit);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("days", kDays, semantic_json));
}

auto HandleDaysDurationQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                             bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  const auto kRows = QueryDayDurations(db_conn, base_filters);
  return BuildSuccessOutput(data_query_renderers::RenderDayDurationsOutput(
      "days_duration", kRows, semantic_json));
}

auto HandleSearchQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       bool semantic_json)
    -> tracer_core::core::dto::TextOutput {
  const auto kItems = QueryDatesByFilters(db_conn, base_filters);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("search", kItems, semantic_json));
}

auto HandleActivitySuggestQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    bool semantic_json) -> tracer_core::core::dto::TextOutput {
  ActivitySuggestionQueryOptions options;
  options.lookback_days =
      request.lookback_days.value_or(kDefaultSuggestLookbackDays);
  options.limit =
      request.top_n.value_or(request.limit.value_or(kDefaultSuggestLimit));
  options.prefix = request.activity_prefix;
  options.score_by_duration = request.activity_score_by_duration;
  const auto kRows = QueryActivitySuggestions(db_conn, options);
  return BuildSuccessOutput(
      data_query_renderers::RenderActivitySuggestionsOutput(kRows,
                                                            semantic_json));
}

}  // namespace tracer_core::infrastructure::query::data::orchestrators
