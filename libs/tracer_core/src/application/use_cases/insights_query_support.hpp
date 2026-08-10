#ifndef APPLICATION_USE_CASES_INSIGHTS_QUERY_SUPPORT_HPP_
#define APPLICATION_USE_CASES_INSIGHTS_QUERY_SUPPORT_HPP_

#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "application/dto/insights_requests.hpp"
#include "application/dto/insights_responses.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "application/ports/insights/i_insights_dto_formatter.hpp"
#include "application/use_cases/insights_api_support.hpp"

namespace tracer::core::application::use_cases::insights_query_support {

struct RecentSelection {
  int days = 0;
  std::optional<std::string> anchor_date;
};

auto BuildTemporalStructuredInsightsFailure(
    std::string_view operation,
    tracer_core::core::dto::InsightsDisplayMode display_mode,
    tracer_core::core::dto::TemporalSelectionKind selection_kind,
    std::string_view details)
    -> tracer_core::core::dto::TemporalStructuredInsightsOutput;
auto BuildTemporalStructuredInsightsFailure(
    std::string_view operation,
    const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest& request,
    std::string_view details)
    -> tracer_core::core::dto::TemporalStructuredInsightsOutput;
auto BuildTemporalStructuredInsightsFailure(
    std::string_view operation,
    const tracer_core::core::dto::TemporalStructuredInsightsQueryRequest& request,
    const std::exception& exception)
    -> tracer_core::core::dto::TemporalStructuredInsightsOutput;

auto BuildTemporalTargetsFailure(
    std::string_view operation,
    tracer_core::core::dto::InsightsDisplayMode display_mode,
    std::string_view details)
    -> tracer_core::core::dto::TemporalInsightsTargetsOutput;

auto FormatTemporalStructuredInsights(
    const tracer_core::core::dto::TemporalStructuredInsightsOutput& output,
    InsightsFormat format, std::string_view locale,
    tracer_core::application::ports::IInsightsDtoFormatter& formatter)
    -> tracer_core::core::dto::TextOutput;

auto NormalizeDateArgument(std::string_view argument) -> std::string;
auto NormalizeMonthArgument(std::string_view argument) -> std::string;
auto ResolveMonthRange(std::string_view month_value)
    -> tracer_core::core::dto::TemporalSelectionPayload;
auto ResolveWeekRange(std::string_view week_value)
    -> tracer_core::core::dto::TemporalSelectionPayload;
auto ResolveYearRange(std::string_view year_value)
    -> tracer_core::core::dto::TemporalSelectionPayload;

auto RequireSingleDaySelection(
    const tracer_core::core::dto::TemporalSelectionPayload& selection)
    -> std::string;
auto RequireDateRangeSelection(
    const tracer_core::core::dto::TemporalSelectionPayload& selection)
    -> tracer::core::application::use_cases::insights_support::DateRangeArgument;
auto RequireRecentSelection(
    const tracer_core::core::dto::TemporalSelectionPayload& selection)
    -> RecentSelection;

auto ResolveAnchoredRecentInsights(
    tracer_core::application::ports::IInsightsDataQueryService& service,
    const RecentSelection& selection) -> PeriodInsightsData;
auto ToPeriodInsights(const RangeInsightsData& source) -> PeriodInsightsData;

}  // namespace tracer::core::application::use_cases::insights_query_support

#endif  // APPLICATION_USE_CASES_INSIGHTS_QUERY_SUPPORT_HPP_
