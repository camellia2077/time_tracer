#ifndef APPLICATION_USE_CASES_REPORT_QUERY_SUPPORT_HPP_
#define APPLICATION_USE_CASES_REPORT_QUERY_SUPPORT_HPP_

#include <exception>
#include <optional>
#include <string>
#include <string_view>

#include "application/dto/reporting_requests.hpp"
#include "application/dto/reporting_responses.hpp"
#include "application/ports/reporting/i_report_data_query_service.hpp"
#include "application/ports/reporting/i_report_dto_formatter.hpp"
#include "application/use_cases/report_api_support.hpp"

namespace tracer::core::application::use_cases::report_query_support {

struct RecentSelection {
  int days = 0;
  std::optional<std::string> anchor_date;
};

auto BuildTemporalStructuredReportFailure(
    std::string_view operation,
    tracer_core::core::dto::ReportDisplayMode display_mode,
    tracer_core::core::dto::TemporalSelectionKind selection_kind,
    std::string_view details)
    -> tracer_core::core::dto::TemporalStructuredReportOutput;
auto BuildTemporalStructuredReportFailure(
    std::string_view operation,
    const tracer_core::core::dto::TemporalStructuredReportQueryRequest& request,
    std::string_view details)
    -> tracer_core::core::dto::TemporalStructuredReportOutput;
auto BuildTemporalStructuredReportFailure(
    std::string_view operation,
    const tracer_core::core::dto::TemporalStructuredReportQueryRequest& request,
    const std::exception& exception)
    -> tracer_core::core::dto::TemporalStructuredReportOutput;

auto BuildTemporalTargetsFailure(
    std::string_view operation,
    tracer_core::core::dto::ReportDisplayMode display_mode,
    std::string_view details)
    -> tracer_core::core::dto::TemporalReportTargetsOutput;

auto FormatTemporalStructuredReport(
    const tracer_core::core::dto::TemporalStructuredReportOutput& output,
    ReportFormat format, std::string_view locale,
    tracer_core::application::ports::IReportDtoFormatter& formatter)
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
    -> tracer::core::application::use_cases::report_support::DateRangeArgument;
auto RequireRecentSelection(
    const tracer_core::core::dto::TemporalSelectionPayload& selection)
    -> RecentSelection;

auto ResolveAnchoredRecentReport(
    tracer_core::application::ports::IReportDataQueryService& service,
    const RecentSelection& selection) -> PeriodReportData;
auto ToPeriodReport(const RangeReportData& source) -> PeriodReportData;

}  // namespace tracer::core::application::use_cases::report_query_support

#endif  // APPLICATION_USE_CASES_REPORT_QUERY_SUPPORT_HPP_
