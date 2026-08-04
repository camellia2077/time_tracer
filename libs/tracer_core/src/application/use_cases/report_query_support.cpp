#include "application/use_cases/report_query_support.hpp"

#include <chrono>
#include <sstream>
#include <stdexcept>

#include "application/use_cases/core_api_failure.hpp"
#include "domain/utils/time_utils.hpp"
#include "shared/types/reporting_errors.hpp"
#include "shared/utils/period_utils.hpp"

namespace tracer::core::application::use_cases::report_query_support {

using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::TemporalReportTargetsOutput;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredReportOutput;
using tracer_core::core::dto::TemporalStructuredReportQueryRequest;
using tracer_core::core::dto::TextOutput;
namespace core_api_failure = tracer::core::application::use_cases::failure;
namespace report_api_support =
    tracer::core::application::use_cases::report_support;

namespace {

auto BuildWindowMetadata(const PeriodReportData& report)
    -> tracer_core::core::dto::ReportWindowMetadata {
  return {.has_records = report.has_records,
          .matched_day_count = report.matched_day_count,
          .matched_record_count = report.matched_record_count,
          .start_date = report.start_date,
          .end_date = report.end_date,
          .requested_days = report.requested_days};
}

auto CopyRangeFields(const RangeReportData& source, RangeReportData& target)
    -> void {
  target.range_label = source.range_label;
  target.start_date = source.start_date;
  target.end_date = source.end_date;
  target.has_records = source.has_records;
  target.matched_day_count = source.matched_day_count;
  target.matched_record_count = source.matched_record_count;
  target.requested_days = source.requested_days;
  target.total_duration = source.total_duration;
  target.actual_days = source.actual_days;
  target.status_true_days = source.status_true_days;
  target.exercise_true_days = source.exercise_true_days;
  target.cardio_true_days = source.cardio_true_days;
  target.anaerobic_true_days = source.anaerobic_true_days;
  target.is_valid = source.is_valid;
  target.project_stats = source.project_stats;
  target.project_tree = source.project_tree;
}

auto WrapMonthlyReport(const PeriodReportData& source) -> MonthlyReportData {
  MonthlyReportData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && out.start_date.size() >= 7U) {
    out.range_label = out.start_date.substr(0, 7);
  }
  return out;
}

auto WrapWeeklyReport(const PeriodReportData& source) -> WeeklyReportData {
  WeeklyReportData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && !out.start_date.empty()) {
    out.range_label = FormatIsoWeek(IsoWeekFromDate(out.start_date));
  }
  return out;
}

auto WrapYearlyReport(const PeriodReportData& source) -> YearlyReportData {
  YearlyReportData out;
  CopyRangeFields(source, out);
  if (out.range_label.empty() && out.start_date.size() >= 4U) {
    out.range_label = out.start_date.substr(0, 4);
  }
  return out;
}

auto ParseIsoDate(std::string_view value) -> std::chrono::sys_days {
  const std::string kNormalized = NormalizeDateArgument(value);
  const int kYear = std::stoi(kNormalized.substr(0, 4));
  const unsigned kMonth =
      static_cast<unsigned>(std::stoi(kNormalized.substr(5, 2)));
  const unsigned kDay =
      static_cast<unsigned>(std::stoi(kNormalized.substr(8, 2)));
  const std::chrono::year_month_day kYmd{std::chrono::year(kYear),
                                        std::chrono::month(kMonth),
                                        std::chrono::day(kDay)};
  if (!kYmd.ok()) {
    throw std::invalid_argument("Invalid ISO date: " + kNormalized);
  }
  return std::chrono::sys_days(kYmd);
}

auto FormatIsoDate(std::chrono::sys_days day) -> std::string {
  const std::chrono::year_month_day kYmd(day);
  std::ostringstream out;
  out << static_cast<int>(kYmd.year()) << '-';
  const unsigned kMonth = static_cast<unsigned>(kYmd.month());
  if (kMonth < 10U) {
    out << '0';
  }
  out << kMonth << '-';
  const unsigned kDayNumber = static_cast<unsigned>(kYmd.day());
  if (kDayNumber < 10U) {
    out << '0';
  }
  out << kDayNumber;
  return out.str();
}

}  // namespace

auto ToPeriodReport(const RangeReportData& source) -> PeriodReportData {
  PeriodReportData out;
  CopyRangeFields(source, out);
  return out;
}

auto BuildTemporalStructuredReportFailure(std::string_view operation,
                                          ReportDisplayMode display_mode,
                                          TemporalSelectionKind selection_kind,
                                          std::string_view details)
    -> TemporalStructuredReportOutput {
  return {
      .ok = false,
      .display_mode = display_mode,
      .selection_kind = selection_kind,
      .report = DailyReportData{},
      .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto BuildTemporalStructuredReportFailure(
    std::string_view operation,
    const TemporalStructuredReportQueryRequest& request,
    std::string_view details) -> TemporalStructuredReportOutput {
  return BuildTemporalStructuredReportFailure(operation, request.display_mode,
                                              request.selection.kind, details);
}

auto BuildTemporalStructuredReportFailure(
    std::string_view operation,
    const TemporalStructuredReportQueryRequest& request,
    const std::exception& exception) -> TemporalStructuredReportOutput {
  return BuildTemporalStructuredReportFailure(operation, request,
                                              exception.what());
}

auto BuildTemporalTargetsFailure(std::string_view operation,
                                 ReportDisplayMode display_mode,
                                 std::string_view details)
    -> TemporalReportTargetsOutput {
  return {
      .ok = false,
      .display_mode = display_mode,
      .items = {},
      .error_message = core_api_failure::BuildErrorMessage(operation, details)};
}

auto FormatTemporalStructuredReport(
    const TemporalStructuredReportOutput& output, ReportFormat format,
    std::string_view locale,
    tracer_core::application::ports::IReportDtoFormatter& formatter)
    -> TextOutput {
  switch (output.display_mode) {
    case ReportDisplayMode::kDay: {
      const auto* report = std::get_if<DailyReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalReportQuery",
            "Temporal structured report kind/data mismatch: day.");
      }
      return {
          .ok = true,
          .content = formatter.FormatDailyLocalized(*report, format, locale),
          .error_message = ""};
    }
    case ReportDisplayMode::kMonth: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalReportQuery",
            "Temporal structured report kind/data mismatch: month.");
      }
      return {.ok = true,
              .content = formatter.FormatMonthlyLocalized(
                  WrapMonthlyReport(*report), format, locale),
              .error_message = ""};
    }
    case ReportDisplayMode::kWeek: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalReportQuery",
            "Temporal structured report kind/data mismatch: week.");
      }
      return {.ok = true,
              .content = formatter.FormatWeeklyLocalized(
                  WrapWeeklyReport(*report), format, locale),
              .error_message = ""};
    }
    case ReportDisplayMode::kYear: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalReportQuery",
            "Temporal structured report kind/data mismatch: year.");
      }
      return {.ok = true,
              .content = formatter.FormatYearlyLocalized(
                  WrapYearlyReport(*report), format, locale),
              .error_message = ""};
    }
    case ReportDisplayMode::kRecent:
    case ReportDisplayMode::kRange: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return core_api_failure::BuildTextFailure(
            "RunTemporalReportQuery",
            "Temporal structured report kind/data mismatch: period.");
      }
      return {
          .ok = true,
          .content = formatter.FormatPeriodLocalized(*report, format, locale),
          .error_message = "",
          .report_window_metadata = BuildWindowMetadata(*report)};
    }
  }
  return core_api_failure::BuildTextFailure(
      "RunTemporalReportQuery",
      "Unhandled temporal structured report display mode.");
}

auto NormalizeDateArgument(std::string_view argument) -> std::string {
  return NormalizeToDateFormat(std::string(argument));
}

auto NormalizeMonthArgument(std::string_view argument) -> std::string {
  return NormalizeToMonthFormat(std::string(argument));
}

auto ResolveMonthRange(std::string_view month_value)
    -> TemporalSelectionPayload {
  const std::string kNormalized = NormalizeMonthArgument(month_value);
  if (kNormalized.size() != 7U) {
    throw std::invalid_argument("Month argument must normalize to YYYY-MM.");
  }
  const int kYear = std::stoi(kNormalized.substr(0, 4));
  const unsigned kMonth =
      static_cast<unsigned>(std::stoi(kNormalized.substr(5, 2)));
  const auto kLastDay = std::chrono::year_month_day_last(
      std::chrono::year(kYear),
      std::chrono::month_day_last(std::chrono::month(kMonth)));
  std::ostringstream end_date;
  end_date << kYear << '-';
  if (kMonth < 10U) {
    end_date << '0';
  }
  end_date << kMonth << '-';
  const unsigned kLastDayNumber = static_cast<unsigned>(kLastDay.day());
  if (kLastDayNumber < 10U) {
    end_date << '0';
  }
  end_date << kLastDayNumber;
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = kNormalized + "-01",
          .end_date = end_date.str()};
}

auto ResolveWeekRange(std::string_view week_value) -> TemporalSelectionPayload {
  IsoWeek week{};
  if (!ParseIsoWeek(week_value, week)) {
    throw std::invalid_argument(
        "Week argument must be in ISO week format (YYYY-Www or YYYYWww).");
  }
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = IsoWeekStartDate(week),
          .end_date = IsoWeekEndDate(week)};
}

auto ResolveYearRange(std::string_view year_value) -> TemporalSelectionPayload {
  int year = 0;
  if (!ParseGregorianYear(year_value, year)) {
    throw std::invalid_argument("Year argument must be YYYY.");
  }
  return {.kind = TemporalSelectionKind::kDateRange,
          .start_date = std::to_string(year) + "-01-01",
          .end_date = std::to_string(year) + "-12-31"};
}

auto RequireSingleDaySelection(const TemporalSelectionPayload& selection)
    -> std::string {
  if (selection.kind != TemporalSelectionKind::kSingleDay) {
    throw tracer_core::common::ReportingContractError(
        "Temporal selection must be single_day for day display mode.",
        "reporting.invalid_selection", "reporting",
        {"Provide a single-day temporal selection for day reports."});
  }
  if (selection.anchor_date.has_value()) {
    throw tracer_core::common::ReportingContractError(
        "anchor_date is only supported for recent_days selection.",
        "reporting.invalid_selection", "reporting",
        {"Remove anchor_date or use recent display mode with recent_days."});
  }
  return NormalizeDateArgument(selection.date);
}

auto RequireDateRangeSelection(const TemporalSelectionPayload& selection)
    -> report_api_support::DateRangeArgument {
  if (selection.kind != TemporalSelectionKind::kDateRange) {
    throw tracer_core::common::ReportingContractError(
        "Temporal selection must be date_range for this display mode.",
        "reporting.invalid_selection", "reporting",
        {"Provide a date-range temporal selection."});
  }
  if (selection.anchor_date.has_value()) {
    throw tracer_core::common::ReportingContractError(
        "anchor_date is only supported for recent_days selection.",
        "reporting.invalid_selection", "reporting",
        {"Remove anchor_date or use recent display mode with recent_days."});
  }
  return report_api_support::ParseRangeArgument(selection.start_date + "|" +
                                                selection.end_date);
}

auto RequireRecentSelection(const TemporalSelectionPayload& selection)
    -> RecentSelection {
  if (selection.kind != TemporalSelectionKind::kRecentDays) {
    throw tracer_core::common::ReportingContractError(
        "Temporal selection must be recent_days for recent display mode.",
        "reporting.invalid_selection", "reporting",
        {"Provide a recent-days temporal selection for recent reports."});
  }
  RecentSelection out{.days = report_api_support::ParseRecentDaysArgument(
                          std::to_string(selection.days)),
                      .anchor_date = std::nullopt};
  if (selection.anchor_date.has_value() && !selection.anchor_date->empty()) {
    out.anchor_date = NormalizeDateArgument(*selection.anchor_date);
  }
  return out;
}

auto ResolveAnchoredRecentReport(
    tracer_core::application::ports::IReportDataQueryService& service,
    const RecentSelection& selection) -> PeriodReportData {
  const auto kAnchorDay = ParseIsoDate(*selection.anchor_date);
  const auto kStartDay = kAnchorDay - std::chrono::days(selection.days - 1);
  PeriodReportData report =
      service.QueryRange(FormatIsoDate(kStartDay), FormatIsoDate(kAnchorDay));
  report.range_label = std::to_string(selection.days) + " days";
  report.start_date = FormatIsoDate(kStartDay);
  report.end_date = FormatIsoDate(kAnchorDay);
  report.requested_days = selection.days;
  return report;
}

}  // namespace tracer::core::application::use_cases::report_query_support
