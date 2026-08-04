#include "application/use_cases/report_api.hpp"

#include <exception>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/report_api_support.hpp"
#include "application/use_cases/report_query_support.hpp"
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

auto ResolveExportPath(const TemporalReportExportRequest& request,
                       const TemporalSelectionPayload& selection) -> fs::path {
  const fs::path kExportRoot = fs::absolute(request.output_root_path);
  switch (request.display_mode) {
    case ReportDisplayMode::kDay:
      return report_api_support::BuildDayPath(
          kExportRoot, request.format,
          report_query_support::RequireSingleDaySelection(selection));
    case ReportDisplayMode::kMonth: {
      const auto kRange =
          report_query_support::RequireDateRangeSelection(selection);
      return report_api_support::BuildMonthPath(kExportRoot, request.format,
                                                kRange.start_date.substr(0, 7));
    }
    case ReportDisplayMode::kRecent:
      return report_api_support::BuildRecentPath(
          kExportRoot, request.format,
          report_query_support::RequireRecentSelection(selection).days);
    case ReportDisplayMode::kWeek: {
      const auto kRange =
          report_query_support::RequireDateRangeSelection(selection);
      return report_api_support::BuildWeekPath(
          kExportRoot, request.format,
          FormatIsoWeek(IsoWeekFromDate(kRange.start_date)));
    }
    case ReportDisplayMode::kYear: {
      const auto kRange =
          report_query_support::RequireDateRangeSelection(selection);
      return report_api_support::BuildYearPath(kExportRoot, request.format,
                                               kRange.start_date.substr(0, 4));
    }
    case ReportDisplayMode::kRange: {
      const auto kRange =
          report_query_support::RequireDateRangeSelection(selection);
      return report_api_support::BuildRangePath(
          kExportRoot, request.format, kRange.start_date, kRange.end_date);
    }
  }
  throw std::invalid_argument("Unhandled export display mode.");
}

auto RenderTemporalReportForExport(ReportApi& api,
                                   const TemporalReportExportRequest& request,
                                   const TemporalSelectionPayload& selection)
    -> TextOutput {
  return api.RunTemporalReportQuery({.display_mode = request.display_mode,
                                     .selection = selection,
                                     .format = request.format,
                                     .locale = request.locale});
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
              .date = report_query_support::NormalizeDateArgument(target)};
    case ReportDisplayMode::kMonth:
      return report_query_support::ResolveMonthRange(target);
    case ReportDisplayMode::kWeek:
      return report_query_support::ResolveWeekRange(target);
    case ReportDisplayMode::kYear:
      return report_query_support::ResolveYearRange(target);
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

auto ReportApi::RunTemporalReportQuery(
    const TemporalReportQueryRequest& request) -> TextOutput {
  try {
    if (!report_data_query_service_ || !report_dto_formatter_) {
      return core_api_failure::BuildTextFailure(
          "RunTemporalReportQuery",
          "Report data query service and formatter are required.");
    }

    const auto kStructured = RunTemporalStructuredReportQuery(
        {.display_mode = request.display_mode, .selection = request.selection});
    if (!kStructured.ok) {
      auto failure = core_api_failure::BuildTextFailure(
          "RunTemporalReportQuery", kStructured.error_message);
      failure.error_contract = kStructured.error_contract;
      return failure;
    }
    return report_query_support::FormatTemporalStructuredReport(
        kStructured, request.format, request.locale, *report_dto_formatter_);
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
      return report_query_support::BuildTemporalStructuredReportFailure(
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
                    report_query_support::RequireSingleDaySelection(
                        request.selection)),
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
        const auto kRange =
            report_query_support::RequireDateRangeSelection(request.selection);
        PeriodReportData report{};
        switch (request.display_mode) {
          case ReportDisplayMode::kMonth:
            // date_range is the canonical contract, but month/week/year still
            // resolve through target-based queries so missing-target behavior
            // and formatter semantics stay aligned with their display modes.
            report = report_query_support::ToPeriodReport(
                report_data_query_service_->QueryMonthly(
                    kRange.start_date.substr(0, 7)));
            break;
          case ReportDisplayMode::kWeek:
            report = report_query_support::ToPeriodReport(
                report_data_query_service_->QueryWeekly(
                    FormatIsoWeek(IsoWeekFromDate(kRange.start_date))));
            break;
          case ReportDisplayMode::kYear:
            report = report_query_support::ToPeriodReport(
                report_data_query_service_->QueryYearly(
                    kRange.start_date.substr(0, 4)));
            break;
          case ReportDisplayMode::kRange:
            report = report_data_query_service_->QueryRange(kRange.start_date,
                                                            kRange.end_date);
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
        const auto kRecent =
            report_query_support::RequireRecentSelection(request.selection);
        PeriodReportData report =
            kRecent.anchor_date.has_value()
                ? report_query_support::ResolveAnchoredRecentReport(
                      *report_data_query_service_, kRecent)
                : report_data_query_service_->QueryPeriod(kRecent.days);
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .report = std::move(report),
                .error_message = ""};
      }
    }

    return report_query_support::BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request,
        "Unhandled temporal selection kind.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = report_query_support::BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request, error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return report_query_support::BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request, exception);
  } catch (...) {
    return report_query_support::BuildTemporalStructuredReportFailure(
        "RunTemporalStructuredReportQuery", request,
        "Unknown non-standard exception.");
  }
}

auto ReportApi::RunPeriodBatchQuery(const PeriodBatchQueryRequest& request)
    -> TextOutput {
  try {
    if (report_data_query_service_ && report_dto_formatter_) {
      const auto kStructured =
          RunStructuredPeriodBatchQuery({.kDays = request.days_list});
      if (!kStructured.ok && kStructured.items.empty()) {
        if (!kStructured.error_message.empty()) {
          return {.ok = false,
                  .content = "",
                  .error_message = kStructured.error_message};
        }
        return core_api_failure::BuildTextFailure(
            "RunPeriodBatchQuery",
            "Structured period batch query failed without error message.");
      }

      std::ostringstream output;
      for (size_t index = 0; index < kStructured.items.size(); ++index) {
        if (index > 0) {
          output << "\n" << std::string(kPeriodSeparatorLength, '-') << "\n";
        }

        const auto& item = kStructured.items[index];
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

    for (const int kDays : request.kDays) {
      StructuredPeriodBatchItem item{
          .kDays = kDays,
          .ok = true,
          .report = std::nullopt,
          .error_message = "",
      };
      try {
        item.report = report_data_query_service_->QueryPeriod(kDays);
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
    const TemporalReportTargetsRequest& request)
    -> TemporalReportTargetsOutput {
  try {
    if (!report_data_query_service_) {
      return report_query_support::BuildTemporalTargetsFailure(
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

    return report_query_support::BuildTemporalTargetsFailure(
        "RunTemporalReportTargetsQuery", request.display_mode,
        "Unhandled report display mode.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = report_query_support::BuildTemporalTargetsFailure(
        "RunTemporalReportTargetsQuery", request.display_mode, error.what());
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return report_query_support::BuildTemporalTargetsFailure(
        "RunTemporalReportTargetsQuery", request.display_mode,
        exception.what());
  } catch (...) {
    return report_query_support::BuildTemporalTargetsFailure(
        "RunTemporalReportTargetsQuery", request.display_mode,
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
        const auto kRendered =
            RenderTemporalReportForExport(*this, request, selection);
        if (!kRendered.ok) {
          return {.ok = false,
                  .error_message = kRendered.error_message,
                  .error_contract = kRendered.error_contract};
        }
        report_api_support::WriteExportFileIfNeeded(
            ResolveExportPath(request, selection), kRendered.content);
        return {.ok = true, .error_message = ""};
      }
      case ReportExportScope::kAllMatching: {
        const auto kTargets = RunTemporalReportTargetsQuery(
            {.display_mode = request.display_mode});
        if (!kTargets.ok) {
          return {.ok = false,
                  .error_message = kTargets.error_message,
                  .error_contract = kTargets.error_contract};
        }
        for (const auto& target : kTargets.items) {
          const auto kSelection =
              BuildSelectionFromTarget(request.display_mode, target);
          const auto kRendered =
              RenderTemporalReportForExport(*this, request, kSelection);
          if (!kRendered.ok) {
            return {.ok = false,
                    .error_message = kRendered.error_message,
                    .error_contract = kRendered.error_contract};
          }
          report_api_support::WriteExportFileIfNeeded(
              ResolveExportPath(request, kSelection), kRendered.content);
        }
        return {.ok = true, .error_message = ""};
      }
      case ReportExportScope::kBatchRecentList:
        for (const int kDays : request.recent_days_list) {
          TemporalSelectionPayload selection{
              .kind = TemporalSelectionKind::kRecentDays, .days = kDays};
          const auto kRendered =
              RenderTemporalReportForExport(*this, request, selection);
          if (!kRendered.ok) {
            return {.ok = false,
                    .error_message = kRendered.error_message,
                    .error_contract = kRendered.error_contract};
          }
          report_api_support::WriteExportFileIfNeeded(
              ResolveExportPath(request, selection), kRendered.content);
        }
        return {.ok = true, .error_message = ""};
    }
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = core_api_failure::BuildOperationFailure(
        "RunTemporalReportExport", error);
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
