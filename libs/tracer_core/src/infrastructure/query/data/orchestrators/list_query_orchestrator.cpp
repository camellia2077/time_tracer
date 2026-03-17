// infrastructure/query/data/orchestrators/list_query_orchestrator.cpp
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.renderers;

#include "infrastructure/query/data/orchestrators/list_query_orchestrator.hpp"

#include <utility>

namespace query_data_repository = tracer::core::infrastructure::query::data;
namespace data_query_renderers =
    tracer::core::infrastructure::query::data::renderers;

namespace tracer::core::infrastructure::query::data::orchestrators {
namespace {

constexpr int kDefaultSuggestLookbackDays = 10;
constexpr int kDefaultSuggestLimit = 5;

auto BuildSuccessOutput(std::string content)
    -> tracer_core::core::dto::TextOutput {
  return {.ok = true, .content = std::move(content), .error_message = ""};
}

}  // namespace

auto HandleYearsQuery(sqlite3* db_conn,
                      tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  const auto kYears = query_data_repository::QueryYears(db_conn);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("years", kYears, output_mode));
}

auto HandleMonthsQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  const auto kMonths =
      query_data_repository::QueryMonths(db_conn, base_filters.kYear);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("months", kMonths, output_mode));
}

auto HandleDaysQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                     tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  const auto kDays = query_data_repository::QueryDays(
      db_conn, base_filters.kYear, base_filters.kMonth, base_filters.from_date,
      base_filters.to_date, base_filters.reverse, base_filters.limit);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("days", kDays, output_mode));
}

auto HandleDaysDurationQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                             tracer_core::core::dto::DataQueryOutputMode
                                 output_mode)
    -> tracer_core::core::dto::TextOutput {
  const auto kRows =
      query_data_repository::QueryDayDurations(db_conn, base_filters);
  return BuildSuccessOutput(data_query_renderers::RenderDayDurationsOutput(
      "days_duration", kRows, output_mode));
}

auto HandleSearchQuery(sqlite3* db_conn, const QueryFilters& base_filters,
                       tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  const auto kItems =
      query_data_repository::QueryDatesByFilters(db_conn, base_filters);
  return BuildSuccessOutput(
      data_query_renderers::RenderListOutput("search", kItems, output_mode));
}

auto HandleActivitySuggestQuery(
    sqlite3* db_conn, const tracer_core::core::dto::DataQueryRequest& request,
    tracer_core::core::dto::DataQueryOutputMode output_mode)
    -> tracer_core::core::dto::TextOutput {
  query_data_repository::ActivitySuggestionQueryOptions options;
  options.lookback_days =
      request.lookback_days.value_or(kDefaultSuggestLookbackDays);
  options.limit =
      request.top_n.value_or(request.limit.value_or(kDefaultSuggestLimit));
  options.prefix = request.activity_prefix;
  options.score_by_duration = request.activity_score_by_duration;
  const auto kRows =
      query_data_repository::QueryActivitySuggestions(db_conn, options);
  return BuildSuccessOutput(
      data_query_renderers::RenderActivitySuggestionsOutput(kRows, output_mode));
}

}  // namespace tracer::core::infrastructure::query::data::orchestrators
