#include "application/use_cases/insights_api.hpp"

#include <exception>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/insights_api_support.hpp"
#include "application/use_cases/insights_query_support.hpp"
#include "shared/types/insights_errors.hpp"
#include "shared/utils/period_utils.hpp"

namespace tracer::core::application::use_cases {

using tracer_core::core::dto::OperationAck;
using tracer_core::core::dto::PeriodBatchQueryRequest;
using tracer_core::core::dto::InsightsDisplayMode;
using tracer_core::core::dto::InsightsExportScope;
using tracer_core::core::dto::StructuredPeriodBatchItem;
using tracer_core::core::dto::StructuredPeriodBatchOutput;
using tracer_core::core::dto::StructuredPeriodBatchQueryRequest;
using tracer_core::core::dto::TemporalInsightsExportRequest;
using tracer_core::core::dto::TemporalInsightsQueryRequest;
using tracer_core::core::dto::TemporalInsightsTargetsOutput;
using tracer_core::core::dto::TemporalInsightsTargetsRequest;
using tracer_core::core::dto::TemporalSelectionKind;
using tracer_core::core::dto::TemporalSelectionPayload;
using tracer_core::core::dto::TemporalStructuredInsightsOutput;
using tracer_core::core::dto::TemporalStructuredInsightsQueryRequest;
using tracer_core::core::dto::TextOutput;
namespace core_api_failure = tracer::core::application::use_cases::failure;
namespace insights_api_support =
    tracer::core::application::use_cases::insights_support;

namespace {

namespace fs = std::filesystem;

constexpr int kPeriodSeparatorLength = 40;

auto ResolveExportPath(const TemporalInsightsExportRequest& request,
                       const TemporalSelectionPayload& selection) -> fs::path {
  const fs::path kExportRoot = fs::absolute(request.output_root_path);
  switch (request.display_mode) {
    case InsightsDisplayMode::kDay:
      return insights_api_support::BuildDayPath(
          kExportRoot, request.format,
          insights_query_support::RequireSingleDaySelection(selection));
    case InsightsDisplayMode::kMonth: {
      const auto kRange =
          insights_query_support::RequireDateRangeSelection(selection);
      return insights_api_support::BuildMonthPath(kExportRoot, request.format,
                                                kRange.start_date.substr(0, 7));
    }
    case InsightsDisplayMode::kRecent:
      return insights_api_support::BuildRecentPath(
          kExportRoot, request.format,
          insights_query_support::RequireRecentSelection(selection).days);
    case InsightsDisplayMode::kWeek: {
      const auto kRange =
          insights_query_support::RequireDateRangeSelection(selection);
      return insights_api_support::BuildWeekPath(
          kExportRoot, request.format,
          FormatIsoWeek(IsoWeekFromDate(kRange.start_date)));
    }
    case InsightsDisplayMode::kYear: {
      const auto kRange =
          insights_query_support::RequireDateRangeSelection(selection);
      return insights_api_support::BuildYearPath(kExportRoot, request.format,
                                               kRange.start_date.substr(0, 4));
    }
    case InsightsDisplayMode::kRange: {
      const auto kRange =
          insights_query_support::RequireDateRangeSelection(selection);
      return insights_api_support::BuildRangePath(
          kExportRoot, request.format, kRange.start_date, kRange.end_date);
    }
  }
  throw std::invalid_argument("Unhandled export display mode.");
}

auto RenderTemporalInsightsForExport(InsightsApi& api,
                                   const TemporalInsightsExportRequest& request,
                                   const TemporalSelectionPayload& selection)
    -> TextOutput {
  return api.RunTemporalInsightsQuery({.display_mode = request.display_mode,
                                     .selection = selection,
                                     .format = request.format,
                                     .locale = request.locale});
}

void RequireExportScopeRules(const TemporalInsightsExportRequest& request) {
  if (request.output_root_path.empty()) {
    throw tracer_core::common::InsightsContractError(
        "Temporal insights export requires non-empty output_root_path.",
        "insights.invalid_export_request", "insights",
        {"Provide a writable export output root path."});
  }

  switch (request.export_scope) {
    case InsightsExportScope::kSingle:
      if (!request.selection.has_value()) {
        throw tracer_core::common::InsightsContractError(
            "Temporal insights export with single scope requires selection.",
            "insights.invalid_export_request", "insights",
            {"Provide a temporal selection for single insights export."});
      }
      return;
    case InsightsExportScope::kAllMatching:
      if (request.selection.has_value()) {
        throw tracer_core::common::InsightsContractError(
            "Temporal insights export with all_matching scope must not include "
            "selection.",
            "insights.invalid_export_request", "insights",
            {"Remove the selection for all-matching export."});
      }
      if (request.display_mode == InsightsDisplayMode::kRange ||
          request.display_mode == InsightsDisplayMode::kRecent) {
        throw tracer_core::common::InsightsContractError(
            "All-matching export only supports day/week/month/year.",
            "insights.unsupported_display_mode", "insights",
            {"Use single export for range or batch_recent_list for recent."});
      }
      return;
    case InsightsExportScope::kBatchRecentList:
      if (request.display_mode != InsightsDisplayMode::kRecent) {
        throw tracer_core::common::InsightsContractError(
            "Batch recent-list export only supports recent display mode.",
            "insights.unsupported_display_mode", "insights",
            {"Use recent display mode for batch recent-list export."});
      }
      if (!request.recent_days_list.empty()) {
        return;
      }
      throw tracer_core::common::InsightsContractError(
          "Batch recent-list export requires recent_days_list.",
          "insights.invalid_export_request", "insights",
          {"Provide a non-empty recent_days_list for batch export."});
  }
}

auto BuildSelectionFromTarget(InsightsDisplayMode display_mode,
                              std::string_view target)
    -> TemporalSelectionPayload {
  // all_matching export enumerates canonical targets first, then maps each
  // target back into a temporal selection so hosts do not need legacy
  // insights/export request builders.
  switch (display_mode) {
    case InsightsDisplayMode::kDay:
      return {.kind = TemporalSelectionKind::kSingleDay,
              .date = insights_query_support::NormalizeDateArgument(target)};
    case InsightsDisplayMode::kMonth:
      return insights_query_support::ResolveMonthRange(target);
    case InsightsDisplayMode::kWeek:
      return insights_query_support::ResolveWeekRange(target);
    case InsightsDisplayMode::kYear:
      return insights_query_support::ResolveYearRange(target);
    case InsightsDisplayMode::kRange:
    case InsightsDisplayMode::kRecent:
      break;
  }
  throw std::invalid_argument("Targets are unsupported for this display mode.");
}

}  // namespace

InsightsApi::InsightsApi(IInsightsHandler& insights_handler,
                     InsightsDataQueryServicePtr insights_data_query_service,
                     InsightsDtoFormatterPtr insights_dto_formatter)
    : insights_handler_(insights_handler),
      insights_data_query_service_(std::move(insights_data_query_service)),
      insights_dto_formatter_(std::move(insights_dto_formatter)) {}

auto InsightsApi::RunTemporalInsightsQuery(
    const TemporalInsightsQueryRequest& request) -> TextOutput {
  try {
    if (!insights_data_query_service_ || !insights_dto_formatter_) {
      return core_api_failure::BuildTextFailure(
          "RunTemporalInsightsQuery",
          "Insights data query service and formatter are required.");
    }

    const auto kStructured = RunTemporalStructuredInsightsQuery(
        {.display_mode = request.display_mode, .selection = request.selection});
    if (!kStructured.ok) {
      auto failure = core_api_failure::BuildTextFailure(
          "RunTemporalInsightsQuery", kStructured.error_message);
      failure.error_contract = kStructured.error_contract;
      return failure;
    }
    return insights_query_support::FormatTemporalStructuredInsights(
        kStructured, request.format, request.locale, *insights_dto_formatter_);
  } catch (const tracer_core::common::InsightsContractError& error) {
    auto failure =
        core_api_failure::BuildTextFailure("RunTemporalInsightsQuery", error);
    tracer_core::common::ApplyInsightsContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunTemporalInsightsQuery",
                                              exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunTemporalInsightsQuery");
  }
}

auto InsightsApi::RunTemporalStructuredInsightsQuery(
    const TemporalStructuredInsightsQueryRequest& request)
    -> TemporalStructuredInsightsOutput {
  try {
    if (!insights_data_query_service_) {
      return insights_query_support::BuildTemporalStructuredInsightsFailure(
          "RunTemporalStructuredInsightsQuery", request,
          "Insights data query service is not configured.");
    }

    switch (request.selection.kind) {
      case TemporalSelectionKind::kSingleDay:
        if (request.display_mode != InsightsDisplayMode::kDay) {
          throw tracer_core::common::InsightsContractError(
              "single_day selection only supports day display mode.",
              "insights.invalid_selection", "insights",
              {"Use date_range for week/month/year/range or recent_days for "
               "recent."});
        }
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .insights = insights_data_query_service_->QueryDaily(
                    insights_query_support::RequireSingleDaySelection(
                        request.selection)),
                .error_message = ""};
      case TemporalSelectionKind::kDateRange: {
        if (request.display_mode != InsightsDisplayMode::kWeek &&
            request.display_mode != InsightsDisplayMode::kMonth &&
            request.display_mode != InsightsDisplayMode::kYear &&
            request.display_mode != InsightsDisplayMode::kRange) {
          throw tracer_core::common::InsightsContractError(
              "date_range selection only supports week/month/year/range "
              "display modes.",
              "insights.invalid_selection", "insights",
              {"Use single_day for day or recent_days for recent."});
        }
        const auto kRange =
            insights_query_support::RequireDateRangeSelection(request.selection);
        PeriodInsightsData insights{};
        switch (request.display_mode) {
          case InsightsDisplayMode::kMonth:
            // date_range is the canonical contract, but month/week/year still
            // resolve through target-based queries so missing-target behavior
            // and formatter semantics stay aligned with their display modes.
            insights = insights_query_support::ToPeriodInsights(
                insights_data_query_service_->QueryMonthly(
                    kRange.start_date.substr(0, 7)));
            break;
          case InsightsDisplayMode::kWeek:
            insights = insights_query_support::ToPeriodInsights(
                insights_data_query_service_->QueryWeekly(
                    FormatIsoWeek(IsoWeekFromDate(kRange.start_date))));
            break;
          case InsightsDisplayMode::kYear:
            insights = insights_query_support::ToPeriodInsights(
                insights_data_query_service_->QueryYearly(
                    kRange.start_date.substr(0, 4)));
            break;
          case InsightsDisplayMode::kRange:
            insights = insights_data_query_service_->QueryRange(kRange.start_date,
                                                            kRange.end_date);
            break;
          case InsightsDisplayMode::kDay:
          case InsightsDisplayMode::kRecent:
            throw std::logic_error(
                "Unexpected display mode in date-range temporal selection.");
        }
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .insights = std::move(insights),
                .error_message = ""};
      }
      case TemporalSelectionKind::kRecentDays: {
        if (request.display_mode != InsightsDisplayMode::kRecent) {
          throw tracer_core::common::InsightsContractError(
              "recent_days selection only supports recent display mode.",
              "insights.invalid_selection", "insights",
              {"Use date_range for week/month/year/range or single_day for "
               "day."});
        }
        const auto kRecent =
            insights_query_support::RequireRecentSelection(request.selection);
        PeriodInsightsData insights =
            kRecent.anchor_date.has_value()
                ? insights_query_support::ResolveAnchoredRecentInsights(
                      *insights_data_query_service_, kRecent)
                : insights_data_query_service_->QueryPeriod(kRecent.days);
        return {.ok = true,
                .display_mode = request.display_mode,
                .selection_kind = request.selection.kind,
                .insights = std::move(insights),
                .error_message = ""};
      }
    }

    return insights_query_support::BuildTemporalStructuredInsightsFailure(
        "RunTemporalStructuredInsightsQuery", request,
        "Unhandled temporal selection kind.");
  } catch (const tracer_core::common::InsightsContractError& error) {
    auto failure = insights_query_support::BuildTemporalStructuredInsightsFailure(
        "RunTemporalStructuredInsightsQuery", request, error);
    tracer_core::common::ApplyInsightsContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return insights_query_support::BuildTemporalStructuredInsightsFailure(
        "RunTemporalStructuredInsightsQuery", request, exception);
  } catch (...) {
    return insights_query_support::BuildTemporalStructuredInsightsFailure(
        "RunTemporalStructuredInsightsQuery", request,
        "Unknown non-standard exception.");
  }
}

auto InsightsApi::RunPeriodBatchQuery(const PeriodBatchQueryRequest& request)
    -> TextOutput {
  try {
    if (insights_data_query_service_ && insights_dto_formatter_) {
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
        if (!item.ok || !item.insights.has_value()) {
          output << insights_api_support::BuildPeriodBatchErrorLine(
              item.kDays, item.error_message);
          continue;
        }

        try {
          output << insights_dto_formatter_->FormatPeriod(*item.insights,
                                                        request.format);
        } catch (const std::exception& exception) {
          output << insights_api_support::BuildPeriodBatchErrorLine(
              item.kDays, exception.what());
        } catch (...) {
          output << insights_api_support::BuildPeriodBatchErrorLine(
              item.kDays, "Unknown non-standard exception.");
        }
      }

      return {.ok = true, .content = output.str(), .error_message = ""};
    }

    return {.ok = true,
            .content = insights_handler_.RunPeriodQueries(request.days_list,
                                                        request.format),
            .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunPeriodBatchQuery", exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunPeriodBatchQuery");
  }
}

auto InsightsApi::RunStructuredPeriodBatchQuery(
    const StructuredPeriodBatchQueryRequest& request)
    -> StructuredPeriodBatchOutput {
  try {
    if (!insights_data_query_service_) {
      return insights_api_support::BuildStructuredPeriodBatchFailure(
          "RunStructuredPeriodBatchQuery",
          "Insights data query service is not configured.");
    }

    StructuredPeriodBatchOutput output{
        .ok = true, .items = {}, .error_message = ""};
    output.items.reserve(request.kDays.size());

    for (const int kDays : request.kDays) {
      StructuredPeriodBatchItem item{
          .kDays = kDays,
          .ok = true,
          .insights = std::nullopt,
          .error_message = "",
      };
      try {
        item.insights = insights_data_query_service_->QueryPeriod(kDays);
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
    return insights_api_support::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery", exception);
  } catch (...) {
    return insights_api_support::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery");
  }
}

auto InsightsApi::RunTemporalInsightsTargetsQuery(
    const TemporalInsightsTargetsRequest& request)
    -> TemporalInsightsTargetsOutput {
  try {
    if (!insights_data_query_service_) {
      return insights_query_support::BuildTemporalTargetsFailure(
          "RunTemporalInsightsTargetsQuery", request.display_mode,
          "Insights data query service is not configured.");
    }

    switch (request.display_mode) {
      case InsightsDisplayMode::kDay:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = insights_data_query_service_->ListDailyTargets(),
                .error_message = ""};
      case InsightsDisplayMode::kMonth:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = insights_data_query_service_->ListMonthlyTargets(),
                .error_message = ""};
      case InsightsDisplayMode::kWeek:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = insights_data_query_service_->ListWeeklyTargets(),
                .error_message = ""};
      case InsightsDisplayMode::kYear:
        return {.ok = true,
                .display_mode = request.display_mode,
                .items = insights_data_query_service_->ListYearlyTargets(),
                .error_message = ""};
      case InsightsDisplayMode::kRange:
      case InsightsDisplayMode::kRecent:
        throw tracer_core::common::InsightsContractError(
            "Insights targets are not supported for range/recent display modes.",
            "insights.unsupported_display_mode", "insights",
            {"Use day, week, month, or year targets instead."});
    }

    return insights_query_support::BuildTemporalTargetsFailure(
        "RunTemporalInsightsTargetsQuery", request.display_mode,
        "Unhandled insights display mode.");
  } catch (const tracer_core::common::InsightsContractError& error) {
    auto failure = insights_query_support::BuildTemporalTargetsFailure(
        "RunTemporalInsightsTargetsQuery", request.display_mode, error.what());
    tracer_core::common::ApplyInsightsContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return insights_query_support::BuildTemporalTargetsFailure(
        "RunTemporalInsightsTargetsQuery", request.display_mode,
        exception.what());
  } catch (...) {
    return insights_query_support::BuildTemporalTargetsFailure(
        "RunTemporalInsightsTargetsQuery", request.display_mode,
        "Unknown non-standard exception.");
  }
}

auto InsightsApi::RunTemporalInsightsExport(
    const TemporalInsightsExportRequest& request) -> OperationAck {
  try {
    RequireExportScopeRules(request);

    switch (request.export_scope) {
      case InsightsExportScope::kSingle: {
        const auto& selection = *request.selection;
        const auto kRendered =
            RenderTemporalInsightsForExport(*this, request, selection);
        if (!kRendered.ok) {
          return {.ok = false,
                  .error_message = kRendered.error_message,
                  .error_contract = kRendered.error_contract};
        }
        insights_api_support::WriteExportFileIfNeeded(
            ResolveExportPath(request, selection), kRendered.content);
        return {.ok = true, .error_message = ""};
      }
      case InsightsExportScope::kAllMatching: {
        const auto kTargets = RunTemporalInsightsTargetsQuery(
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
              RenderTemporalInsightsForExport(*this, request, kSelection);
          if (!kRendered.ok) {
            return {.ok = false,
                    .error_message = kRendered.error_message,
                    .error_contract = kRendered.error_contract};
          }
          insights_api_support::WriteExportFileIfNeeded(
              ResolveExportPath(request, kSelection), kRendered.content);
        }
        return {.ok = true, .error_message = ""};
      }
      case InsightsExportScope::kBatchRecentList:
        for (const int kDays : request.recent_days_list) {
          TemporalSelectionPayload selection{
              .kind = TemporalSelectionKind::kRecentDays, .days = kDays};
          const auto kRendered =
              RenderTemporalInsightsForExport(*this, request, selection);
          if (!kRendered.ok) {
            return {.ok = false,
                    .error_message = kRendered.error_message,
                    .error_contract = kRendered.error_contract};
          }
          insights_api_support::WriteExportFileIfNeeded(
              ResolveExportPath(request, selection), kRendered.content);
        }
        return {.ok = true, .error_message = ""};
    }
  } catch (const tracer_core::common::InsightsContractError& error) {
    auto failure = core_api_failure::BuildOperationFailure(
        "RunTemporalInsightsExport", error);
    tracer_core::common::ApplyInsightsContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunTemporalInsightsExport",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunTemporalInsightsExport");
  }
}

}  // namespace tracer::core::application::use_cases
