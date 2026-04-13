#include "application/use_cases/report_api.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/report_api_support.hpp"
#include "domain/utils/time_utils.hpp"
#include "shared/types/reporting_errors.hpp"
#include "shared/utils/period_utils.hpp"

namespace tracer::core::application::use_cases {

using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::ReportDisplayMode;
using tracer_core::core::dto::ReportExportScope;
using tracer_core::core::dto::StructuredPeriodBatchItem;
using tracer_core::core::dto::StructuredPeriodBatchOutput;
using tracer_core::core::dto::StructuredPeriodBatchQueryRequest;
using tracer_core::core::dto::TemporalReportExportRequest;
using tracer_core::core::dto::TemporalReportQueryRequest;
using tracer_core::core::dto::TemporalReportTargetsOutput;
using tracer_core::core::dto::TemporalReportTargetsRequest;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredReportOutput;
using tracer_core::core::dto::TemporalStructuredReportQueryRequest;
using tracer_core::core::dto::TextOutput;
namespace core_api_failure = tracer::core::application::use_cases::failure;
namespace report_api_support =
    tracer::core::application::use_cases::report_support;

namespace {

namespace fs = std::filesystem;

constexpr int kPeriodSeparatorLength = 40;

struct RecentSelection {
  int days = 0;
  std::optional<std::string> anchor_date;
};

auto BuildTemporalStructuredReportFailure(
    std::string_view operation, ReportDisplayMode display_mode,
    TemporalSelectionKind selection_kind, std::string_view details)
    -> TemporalStructuredReportOutput {
  return {
      .ok = false,
      .display_mode = display_mode,
      .selection_kind = selection_kind,
      .report = DailyReportData{},
      .error_message = core_api_failure::BuildErrorMessage(operation, details),
  };
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
      .error_message = core_api_failure::BuildErrorMessage(operation, details),
  };
}

auto BuildWindowMetadata(const PeriodReportData& report)
    -> tracer_core::core::dto::ReportWindowMetadata {
  return {
      .has_records = report.has_records,
      .matched_day_count = report.matched_day_count,
      .matched_record_count = report.matched_record_count,
      .start_date = report.start_date,
      .end_date = report.end_date,
      .requested_days = report.requested_days,
  };
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
  target.wake_anchor_true_days = source.wake_anchor_true_days;
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

auto ToPeriodReport(const RangeReportData& source) -> PeriodReportData {
  PeriodReportData out;
  CopyRangeFields(source, out);
  return out;
}

auto FormatTemporalStructuredReport(
    const TemporalStructuredReportOutput& output, ReportFormat format,
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
      return {.ok = true,
              .content = formatter.FormatDaily(*report, format),
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
              .content = formatter.FormatMonthly(WrapMonthlyReport(*report),
                                                 format),
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
              .content = formatter.FormatWeekly(WrapWeeklyReport(*report),
                                                format),
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
              .content = formatter.FormatYearly(WrapYearlyReport(*report),
                                                format),
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
      return {.ok = true,
              .content = formatter.FormatPeriod(*report, format),
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

auto ParseIsoDate(std::string_view value) -> std::chrono::sys_days {
  const std::string normalized = NormalizeDateArgument(value);
  const int year = std::stoi(normalized.substr(0, 4));
  const unsigned month =
      static_cast<unsigned>(std::stoi(normalized.substr(5, 2)));
  const unsigned day = static_cast<unsigned>(std::stoi(normalized.substr(8, 2)));
  const std::chrono::year_month_day ymd{
      std::chrono::year(year),
      std::chrono::month(month),
      std::chrono::day(day),
  };
  if (!ymd.ok()) {
    throw std::invalid_argument("Invalid ISO date: " + normalized);
  }
  return std::chrono::sys_days(ymd);
}

auto FormatIsoDate(std::chrono::sys_days day) -> std::string {
  const std::chrono::year_month_day ymd(day);
  std::ostringstream out;
  out << static_cast<int>(ymd.year()) << '-';
  const unsigned month = static_cast<unsigned>(ymd.month());
  if (month < 10U) {
    out << '0';
  }
  out << month << '-';
  const unsigned day_number = static_cast<unsigned>(ymd.day());
  if (day_number < 10U) {
    out << '0';
  }
  out << day_number;
  return out.str();
}

auto ResolveMonthRange(std::string_view month_value) -> TemporalSelectionPayload {
  const std::string normalized = NormalizeMonthArgument(month_value);
  if (normalized.size() != 7U) {
    throw std::invalid_argument("Month argument must normalize to YYYY-MM.");
  }
  const int year = std::stoi(normalized.substr(0, 4));
  const unsigned month =
      static_cast<unsigned>(std::stoi(normalized.substr(5, 2)));
  const auto last_day = std::chrono::year_month_day_last(
      std::chrono::year(year),
      std::chrono::month_day_last(std::chrono::month(month)));
  std::ostringstream end_date;
  end_date << year << '-';
  if (month < 10U) {
    end_date << '0';
  }
  end_date << month << '-';
  const unsigned last_day_number = static_cast<unsigned>(last_day.day());
  if (last_day_number < 10U) {
    end_date << '0';
  }
  end_date << last_day_number;
  return {
      .kind = TemporalSelectionKind::kDateRange,
      .start_date = normalized + "-01",
      .end_date = end_date.str(),
  };
}

auto ResolveWeekRange(std::string_view week_value) -> TemporalSelectionPayload {
  IsoWeek week{};
  if (!ParseIsoWeek(week_value, week)) {
    throw std::invalid_argument(
        "Week argument must be in ISO week format (YYYY-Www or YYYYWww).");
  }
  return {
      .kind = TemporalSelectionKind::kDateRange,
      .start_date = IsoWeekStartDate(week),
      .end_date = IsoWeekEndDate(week),
  };
}

auto ResolveYearRange(std::string_view year_value) -> TemporalSelectionPayload {
  int year = 0;
  if (!ParseGregorianYear(year_value, year)) {
    throw std::invalid_argument("Year argument must be YYYY.");
  }
  return {
      .kind = TemporalSelectionKind::kDateRange,
      .start_date = std::to_string(year) + "-01-01",
      .end_date = std::to_string(year) + "-12-31",
  };
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

  RecentSelection out{
      .days = report_api_support::ParseRecentDaysArgument(
          std::to_string(selection.days)),
      .anchor_date = std::nullopt,
  };
  if (selection.anchor_date.has_value() && !selection.anchor_date->empty()) {
    out.anchor_date = NormalizeDateArgument(*selection.anchor_date);
  }
  return out;
}

auto ResolveAnchoredRecentReport(
    tracer_core::application::ports::IReportDataQueryService& service,
    const RecentSelection& selection) -> PeriodReportData {
  const auto anchor_day = ParseIsoDate(*selection.anchor_date);
  const auto start_day =
      anchor_day - std::chrono::days(selection.days - 1);
  PeriodReportData report =
      service.QueryRange(FormatIsoDate(start_day), FormatIsoDate(anchor_day));
  // Anchored recent reuses the fixed-window range fetch, then restores recent
  // metadata so formatting/export still behaves like recent instead of range.
  report.range_label = std::to_string(selection.days) + " days";
  report.start_date = FormatIsoDate(start_day);
  report.end_date = FormatIsoDate(anchor_day);
  report.requested_days = selection.days;
  return report;
}

auto ExtensionForFormat(ReportFormat format) -> std::string_view {
  switch (format) {
    case ReportFormat::kMarkdown:
      return ".md";
    case ReportFormat::kLaTeX:
      return ".tex";
    case ReportFormat::kTyp:
      return ".typ";
  }
  throw std::invalid_argument("Unsupported report format.");
}

auto DirectoryForFormat(ReportFormat format) -> std::string_view {
  switch (format) {
    case ReportFormat::kMarkdown:
      return "markdown";
    case ReportFormat::kLaTeX:
      return "latex";
    case ReportFormat::kTyp:
      return "typ";
  }
  throw std::invalid_argument("Unsupported report format.");
}

auto ShouldSkipExportWrite(std::string_view content) -> bool {
  return content.empty() ||
         content.find("No time records") != std::string_view::npos;
}

void WriteUtf8File(const fs::path& output_path, std::string_view content) {
  const fs::path parent = output_path.parent_path();
  if (!parent.empty()) {
    fs::create_directories(parent);
  }

  std::ofstream file(output_path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    throw tracer_core::common::ReportingContractError(
        "Unable to open report export file: " + output_path.string(),
        "export.write.failed", "export",
        {"Check that the export output directory is writable."});
  }
  file.write(content.data(), static_cast<std::streamsize>(content.size()));
  if (file.fail()) {
    throw tracer_core::common::ReportingContractError(
        "Failed to write report export file: " + output_path.string(),
        "export.write.failed", "export",
        {"Check that the export output directory is writable."});
  }
}

void WriteExportFileIfNeeded(const fs::path& output_path,
                             std::string_view content) {
  if (ShouldSkipExportWrite(content)) {
    return;
  }
  WriteUtf8File(output_path, content);
}

auto BuildDayPath(const fs::path& export_root, ReportFormat format,
                  std::string_view date) -> fs::path {
  const fs::path base_dir = export_root / DirectoryForFormat(format) / "day";
  if (date.size() == 10U) {
    return base_dir / std::string(date.substr(0, 4)) /
           std::string(date.substr(5, 2)) /
           (std::string(date) + std::string(ExtensionForFormat(format)));
  }
  return base_dir /
         (std::string(date) + std::string(ExtensionForFormat(format)));
}

auto BuildMonthPath(const fs::path& export_root, ReportFormat format,
                    std::string_view month) -> fs::path {
  return export_root / DirectoryForFormat(format) / "month" /
         (std::string(month) + std::string(ExtensionForFormat(format)));
}

auto BuildRecentPath(const fs::path& export_root, ReportFormat format, int days)
    -> fs::path {
  return export_root / DirectoryForFormat(format) / "recent" /
         ("last_" + std::to_string(days) + "_days_report" +
          std::string(ExtensionForFormat(format)));
}

auto BuildWeekPath(const fs::path& export_root, ReportFormat format,
                   std::string_view iso_week) -> fs::path {
  return export_root / DirectoryForFormat(format) / "week" /
         (std::string(iso_week) + std::string(ExtensionForFormat(format)));
}

auto BuildYearPath(const fs::path& export_root, ReportFormat format,
                   std::string_view year) -> fs::path {
  return export_root / DirectoryForFormat(format) / "year" /
         (std::string(year) + std::string(ExtensionForFormat(format)));
}

auto BuildRangePath(const fs::path& export_root, ReportFormat format,
                    std::string_view start_date, std::string_view end_date)
    -> fs::path {
  return export_root / DirectoryForFormat(format) / "range" /
         (std::string(start_date) + "_" + std::string(end_date) +
          std::string(ExtensionForFormat(format)));
}

auto ResolveExportPath(const TemporalReportExportRequest& request,
                       const TemporalSelectionPayload& selection) -> fs::path {
  const fs::path export_root = fs::absolute(request.output_root_path);
  switch (request.display_mode) {
    case ReportDisplayMode::kDay:
      return BuildDayPath(export_root, request.format,
                          RequireSingleDaySelection(selection));
    case ReportDisplayMode::kMonth: {
      const auto range = RequireDateRangeSelection(selection);
      return BuildMonthPath(export_root, request.format,
                            range.start_date.substr(0, 7));
    }
    case ReportDisplayMode::kRecent:
      return BuildRecentPath(export_root, request.format,
                             RequireRecentSelection(selection).days);
    case ReportDisplayMode::kWeek: {
      const auto range = RequireDateRangeSelection(selection);
      return BuildWeekPath(export_root, request.format,
                           FormatIsoWeek(IsoWeekFromDate(range.start_date)));
    }
    case ReportDisplayMode::kYear: {
      const auto range = RequireDateRangeSelection(selection);
      return BuildYearPath(export_root, request.format,
                           range.start_date.substr(0, 4));
    }
    case ReportDisplayMode::kRange: {
      const auto range = RequireDateRangeSelection(selection);
      return BuildRangePath(export_root, request.format, range.start_date,
                            range.end_date);
    }
  }
  throw std::invalid_argument("Unhandled export display mode.");
}

auto RenderTemporalReportForExport(ReportApi& api,
                                   const TemporalReportExportRequest& request,
                                   const TemporalSelectionPayload& selection)
    -> TextOutput {
  return api.RunTemporalReportQuery(
      {.display_mode = request.display_mode,
       .selection = selection,
       .format = request.format});
}

void RequireExportScopeRules(const TemporalReportExportRequest& request) {
  if (request.output_root_path.empty()) {
    throw tracer_core::common::ReportingContractError(
        "Temporal report export requires non-empty output_root_path.",
        "reporting.invalid_export_request", "reporting",
        {"Provide a writable export output root path."});
  }

  switch (request.export_scope) {
    case ReportExportScope::kSingle:
      if (!request.selection.has_value()) {
        throw tracer_core::common::ReportingContractError(
            "Temporal report export with single scope requires selection.",
            "reporting.invalid_export_request", "reporting",
            {"Provide a temporal selection for single report export."});
      }
      return;
    case ReportExportScope::kAllMatching:
      if (request.selection.has_value()) {
        throw tracer_core::common::ReportingContractError(
            "Temporal report export with all_matching scope must not include "
            "selection.",
            "reporting.invalid_export_request", "reporting",
            {"Remove the selection for all-matching export."});
      }
      if (request.display_mode == ReportDisplayMode::kRange ||
          request.display_mode == ReportDisplayMode::kRecent) {
        throw tracer_core::common::ReportingContractError(
            "All-matching export only supports day/week/month/year.",
            "reporting.unsupported_display_mode", "reporting",
            {"Use single export for range or batch_recent_list for recent."});
      }
      return;
    case ReportExportScope::kBatchRecentList:
      if (request.display_mode != ReportDisplayMode::kRecent) {
        throw tracer_core::common::ReportingContractError(
            "Batch recent-list export only supports recent display mode.",
            "reporting.unsupported_display_mode", "reporting",
            {"Use recent display mode for batch recent-list export."});
      }
      if (!request.recent_days_list.empty()) {
        return;
      }
      throw tracer_core::common::ReportingContractError(
          "Batch recent-list export requires recent_days_list.",
          "reporting.invalid_export_request", "reporting",
          {"Provide a non-empty recent_days_list for batch export."});
  }
}

auto BuildSelectionFromTarget(ReportDisplayMode display_mode,
                              std::string_view target)
    -> TemporalSelectionPayload {
  // all_matching export enumerates canonical targets first, then maps each
  // target back into a temporal selection so hosts do not need legacy
  // report/export request builders.
  switch (display_mode) {
    case ReportDisplayMode::kDay:
      return {.kind = TemporalSelectionKind::kSingleDay,
              .date = NormalizeDateArgument(target)};
    case ReportDisplayMode::kMonth:
      return ResolveMonthRange(target);
    case ReportDisplayMode::kWeek:
      return ResolveWeekRange(target);
    case ReportDisplayMode::kYear:
      return ResolveYearRange(target);
    case ReportDisplayMode::kRange:
    case ReportDisplayMode::kRecent:
      break;
  }
  throw std::invalid_argument("Targets are unsupported for this display mode.");
}

}  // namespace

ReportApi::ReportApi(IReportHandler& report_handler,
                     ReportDataQueryServicePtr report_data_query_service,
                     ReportDtoFormatterPtr report_dto_formatter)
    : report_handler_(report_handler),
      report_data_query_service_(std::move(report_data_query_service)),
      report_dto_formatter_(std::move(report_dto_formatter)) {}

auto ReportApi::RunTemporalReportQuery(const TemporalReportQueryRequest& request)
    -> TextOutput {
  try {
    if (!report_data_query_service_ || !report_dto_formatter_) {
      return core_api_failure::BuildTextFailure(
          "RunTemporalReportQuery",
          "Report data query service and formatter are required.");
    }

    const auto structured = RunTemporalStructuredReportQuery(
        {.display_mode = request.display_mode, .selection = request.selection});
    if (!structured.ok) {
      return {.ok = false,
              .content = "",
              .error_message = structured.error_message,
              .error_contract = structured.error_contract};
    }
    return FormatTemporalStructuredReport(structured, request.format,
                                          *report_dto_formatter_);
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure =
        core_api_failure::BuildTextFailure("RunTemporalReportQuery", error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunTemporalReportQuery",
                                              exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunTemporalReportQuery");
  }
}

auto ReportApi::RunTemporalStructuredReportQuery(
    const TemporalStructuredReportQueryRequest& request)
    -> TemporalStructuredReportOutput {
  try {
    if (!report_data_query_service_) {
      return BuildTemporalStructuredReportFailure(
          "RunTemporalStructuredReportQuery", request,
          "Report data query service is not configured.");
    }

    switch (request.selection.kind) {
      case TemporalSelectionKind::kSingleDay:
        if (request.display_mode != ReportDisplayMode::kDay) {
          throw tracer_core::common::ReportingContractError(
              "single_day selection only supports day display mode.",
              "reporting.invalid_selection", "reporting",
              {"Use date_range for week/month/year/range or recent_days for "
               "recent."});
        }
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .report = report_data_query_service_->QueryDaily(
                    RequireSingleDaySelection(request.selection)),
                .error_message = ""};
      case TemporalSelectionKind::kDateRange: {
        if (request.display_mode != ReportDisplayMode::kWeek &&
            request.display_mode != ReportDisplayMode::kMonth &&
            request.display_mode != ReportDisplayMode::kYear &&
            request.display_mode != ReportDisplayMode::kRange) {
          throw tracer_core::common::ReportingContractError(
              "date_range selection only supports week/month/year/range "
              "display modes.",
              "reporting.invalid_selection", "reporting",
              {"Use single_day for day or recent_days for recent."});
        }
        const auto range = RequireDateRangeSelection(request.selection);
        PeriodReportData report{};
        switch (request.display_mode) {
          case ReportDisplayMode::kMonth:
            // date_range is the canonical contract, but month/week/year still
            // resolve through target-based queries so missing-target behavior
            // and formatter semantics stay aligned with their display modes.
            report = ToPeriodReport(
                report_data_query_service_->QueryMonthly(
                    range.start_date.substr(0, 7)));
            break;
          case ReportDisplayMode::kWeek:
            report = ToPeriodReport(report_data_query_service_->QueryWeekly(
                FormatIsoWeek(IsoWeekFromDate(range.start_date))));
            break;
          case ReportDisplayMode::kYear:
            report = ToPeriodReport(report_data_query_service_->QueryYearly(
                range.start_date.substr(0, 4)));
            break;
          case ReportDisplayMode::kRange:
            report = report_data_query_service_->QueryRange(range.start_date,
                                                            range.end_date);
            break;
          case ReportDisplayMode::kDay:
          case ReportDisplayMode::kRecent:
            throw std::logic_error(
                "Unexpected display mode in date-range temporal selection.");
        }
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .report = std::move(report),
                .error_message = ""};
      }
      case TemporalSelectionKind::kRecentDays: {
        if (request.display_mode != ReportDisplayMode::kRecent) {
          throw tracer_core::common::ReportingContractError(
              "recent_days selection only supports recent display mode.",
              "reporting.invalid_selection", "reporting",
              {"Use date_range for week/month/year/range or single_day for "
               "day."});
        }
        const auto recent = RequireRecentSelection(request.selection);
        PeriodReportData report = recent.anchor_date.has_value()
                                      ? ResolveAnchoredRecentReport(
                                            *report_data_query_service_, recent)
                                      : report_data_query_service_->QueryPeriod(
                                            recent.days);
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .report = std::move(report),
                .error_message = ""};
      }
    }

    return BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request,
        "Unhandled temporal selection kind.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request, error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request, exception);
  } catch (...) {
    return BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request,
        "Unknown non-standard exception.");
  }
}

auto ReportApi::RunPeriodBatchQuery(const PeriodBatchQueryRequest& request)
    -> TextOutput {
  try {
    if (report_data_query_service_ && report_dto_formatter_) {
      const auto structured =
          RunStructuredPeriodBatchQuery({.kDays = request.days_list});
      if (!structured.ok && structured.items.empty()) {
        if (!structured.error_message.empty()) {
          return {.ok = false,
                  .content = "",
                  .error_message = structured.error_message};
        }
        return core_api_failure::BuildTextFailure(
            "RunPeriodBatchQuery",
            "Structured period batch query failed without error message.");
      }

      std::ostringstream output;
      for (size_t index = 0; index < structured.items.size(); ++index) {
        if (index > 0) {
          output << "\n" << std::string(kPeriodSeparatorLength, '-') << "\n";
        }

        const auto& item = structured.items[index];
        if (!item.ok || !item.report.has_value()) {
          output << report_api_support::BuildPeriodBatchErrorLine(
              item.kDays, item.error_message);
          continue;
        }

        try {
          output << report_dto_formatter_->FormatPeriod(*item.report,
                                                        request.format);
        } catch (const std::exception& exception) {
          output << report_api_support::BuildPeriodBatchErrorLine(
              item.kDays, exception.what());
        } catch (...) {
          output << report_api_support::BuildPeriodBatchErrorLine(
              item.kDays, "Unknown non-standard exception.");
        }
      }

      return {.ok = true, .content = output.str(), .error_message = ""};
    }

    return {.ok = true,
            .content = report_handler_.RunPeriodQueries(request.days_list,
                                                        request.format),
            .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunPeriodBatchQuery", exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunPeriodBatchQuery");
  }
}

auto ReportApi::RunStructuredPeriodBatchQuery(
    const StructuredPeriodBatchQueryRequest& request)
    -> StructuredPeriodBatchOutput {
  try {
    if (!report_data_query_service_) {
      return report_api_support::BuildStructuredPeriodBatchFailure(
          "RunStructuredPeriodBatchQuery",
          "Report data query service is not configured.");
    }

    StructuredPeriodBatchOutput output{
        .ok = true, .items = {}, .error_message = ""};
    output.items.reserve(request.kDays.size());

    for (const int days : request.kDays) {
      StructuredPeriodBatchItem item{
          .kDays = days,
          .ok = true,
          .report = std::nullopt,
          .error_message = "",
      };
      try {
        item.report = report_data_query_service_->QueryPeriod(days);
      } catch (const std::exception& exception) {
        item.ok = false;
        item.error_message = exception.what();
        output.ok = false;
      } catch (...) {
        item.ok = false;
        item.error_message = "Unknown non-standard exception.";
        output.ok = false;
      }
      output.items.push_back(std::move(item));
    }

    return output;
  } catch (const std::exception& exception) {
    return report_api_support::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery", exception);
  } catch (...) {
    return report_api_support::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery");
  }
}

auto ReportApi::RunTemporalReportTargetsQuery(
    const TemporalReportTargetsRequest& request) -> TemporalReportTargetsOutput {
  try {
    if (!report_data_query_service_) {
      return BuildTemporalTargetsFailure(
          "RunTemporalReportTargetsQuery", request.display_mode,
          "Report data query service is not configured.");
    }

    switch (request.display_mode) {
      case ReportDisplayMode::kDay:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = report_data_query_service_->ListDailyTargets(),
                .error_message = ""};
      case ReportDisplayMode::kMonth:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = report_data_query_service_->ListMonthlyTargets(),
                .error_message = ""};
      case ReportDisplayMode::kWeek:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = report_data_query_service_->ListWeeklyTargets(),
                .error_message = ""};
      case ReportDisplayMode::kYear:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = report_data_query_service_->ListYearlyTargets(),
                .error_message = ""};
      case ReportDisplayMode::kRange:
      case ReportDisplayMode::kRecent:
        throw tracer_core::common::ReportingContractError(
            "Report targets are not supported for range/recent display modes.",
            "reporting.unsupported_display_mode", "reporting",
            {"Use day, week, month, or year targets instead."});
    }

    return BuildTemporalTargetsFailure("RunTemporalReportTargetsQuery",
                                       request.display_mode,
                                       "Unhandled report display mode.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = BuildTemporalTargetsFailure("RunTemporalReportTargetsQuery",
                                               request.display_mode, error.what());
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return BuildTemporalTargetsFailure("RunTemporalReportTargetsQuery",
                                       request.display_mode, exception.what());
  } catch (...) {
    return BuildTemporalTargetsFailure("RunTemporalReportTargetsQuery",
                                       request.display_mode,
                                       "Unknown non-standard exception.");
  }
}

auto ReportApi::RunTemporalReportExport(
    const TemporalReportExportRequest& request) -> OperationAck {
  try {
    RequireExportScopeRules(request);

    switch (request.export_scope) {
      case ReportExportScope::kSingle: {
        const auto& selection = *request.selection;
        const auto rendered =
            RenderTemporalReportForExport(*this, request, selection);
        if (!rendered.ok) {
          return {.ok = false,
                  .error_message = rendered.error_message,
                  .error_contract = rendered.error_contract};
        }
        WriteExportFileIfNeeded(ResolveExportPath(request, selection),
                                rendered.content);
        return {.ok = true, .error_message = ""};
      }
      case ReportExportScope::kAllMatching: {
        const auto targets =
            RunTemporalReportTargetsQuery({.display_mode = request.display_mode});
        if (!targets.ok) {
          return {.ok = false,
                  .error_message = targets.error_message,
                  .error_contract = targets.error_contract};
        }
        for (const auto& target : targets.items) {
          const auto selection =
              BuildSelectionFromTarget(request.display_mode, target);
          const auto rendered =
              RenderTemporalReportForExport(*this, request, selection);
          if (!rendered.ok) {
            return {.ok = false,
                    .error_message = rendered.error_message,
                    .error_contract = rendered.error_contract};
          }
          WriteExportFileIfNeeded(ResolveExportPath(request, selection),
                                  rendered.content);
        }
        return {.ok = true, .error_message = ""};
      }
      case ReportExportScope::kBatchRecentList:
        for (const int days : request.recent_days_list) {
          TemporalSelectionPayload selection{
              .kind = TemporalSelectionKind::kRecentDays, .days = days};
          const auto rendered =
              RenderTemporalReportForExport(*this, request, selection);
          if (!rendered.ok) {
            return {.ok = false,
                    .error_message = rendered.error_message,
                    .error_contract = rendered.error_contract};
          }
          WriteExportFileIfNeeded(ResolveExportPath(request, selection),
                                  rendered.content);
        }
        return {.ok = true, .error_message = ""};
    }
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure =
        core_api_failure::BuildOperationFailure("RunTemporalReportExport", error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunTemporalReportExport",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunTemporalReportExport");
  }
}

}  // namespace tracer::core::application::use_cases
