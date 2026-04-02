#include "application/use_cases/report_api.hpp"

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/report_api_support.hpp"
#include "shared/types/reporting_errors.hpp"

namespace tracer::core::application::use_cases {

using namespace tracer_core::core::dto;
namespace core_api_failure = tracer::core::application::use_cases::failure;
namespace report_api_support =
    tracer::core::application::use_cases::report_support;

namespace {

constexpr int kPeriodSeparatorLength = 40;

}  // namespace

ReportApi::ReportApi(IReportHandler& report_handler,
                     ReportDataQueryServicePtr report_data_query_service,
                     ReportDtoFormatterPtr report_dto_formatter)
    : report_handler_(report_handler),
      report_data_query_service_(std::move(report_data_query_service)),
      report_dto_formatter_(std::move(report_dto_formatter)) {}

auto ReportApi::RunReportQuery(const ReportQueryRequest& request)
    -> TextOutput {
  try {
    if (report_data_query_service_ && report_dto_formatter_) {
      const auto structured = RunStructuredReportQuery(
          {.type = request.type, .argument = request.argument});
      if (!structured.ok) {
        if (!structured.error_message.empty()) {
          return {.ok = false,
                  .content = "",
                  .error_message = structured.error_message,
                  .error_contract = structured.error_contract};
        }
        return core_api_failure::BuildTextFailure(
            "RunReportQuery",
            "Structured report query failed without error message.");
      }
      return report_api_support::FormatStructuredReport(
          structured, request.format, *report_dto_formatter_);
    }

    switch (request.type) {
      case ReportQueryType::kDay:
        return {.ok = true,
                .content = report_handler_.RunDailyQuery(request.argument,
                                                         request.format),
                .error_message = ""};
      case ReportQueryType::kMonth:
        return {.ok = true,
                .content = report_handler_.RunMonthlyQuery(request.argument,
                                                           request.format),
                .error_message = ""};
      case ReportQueryType::kRecent: {
        const int days =
            report_api_support::ParseRecentDaysArgument(request.argument);
        return {.ok = true,
                .content = report_handler_.RunPeriodQuery(days, request.format),
                .error_message = ""};
      }
      case ReportQueryType::kRange:
        return core_api_failure::BuildTextFailure(
            "RunReportQuery",
            "Range report requires report data query service and formatter.");
      case ReportQueryType::kWeek:
        return {.ok = true,
                .content = report_handler_.RunWeeklyQuery(request.argument,
                                                          request.format),
                .error_message = ""};
      case ReportQueryType::kYear:
        return {.ok = true,
                .content = report_handler_.RunYearlyQuery(request.argument,
                                                          request.format),
                .error_message = ""};
    }

    return core_api_failure::BuildTextFailure("RunReportQuery",
                                              "Unhandled report query type.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = core_api_failure::BuildTextFailure("RunReportQuery", error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return core_api_failure::BuildTextFailure("RunReportQuery", exception);
  } catch (...) {
    return core_api_failure::BuildTextFailure("RunReportQuery");
  }
}

auto ReportApi::RunStructuredReportQuery(
    const StructuredReportQueryRequest& request) -> StructuredReportOutput {
  try {
    if (!report_data_query_service_) {
      return report_api_support::BuildStructuredReportFailure(
          "RunStructuredReportQuery",
          "Report data query service is not configured.");
    }

    switch (request.type) {
      case ReportQueryType::kDay:
        return {
            .ok = true,
            .kind = StructuredReportKind::kDay,
            .report = report_data_query_service_->QueryDaily(request.argument),
            .error_message = ""};
      case ReportQueryType::kMonth:
        return {.ok = true,
                .kind = StructuredReportKind::kMonth,
                .report =
                    report_data_query_service_->QueryMonthly(request.argument),
                .error_message = ""};
      case ReportQueryType::kRecent: {
        const int days =
            report_api_support::ParseRecentDaysArgument(request.argument);
        return {.ok = true,
                .kind = StructuredReportKind::kRecent,
                .report = report_data_query_service_->QueryPeriod(days),
                .error_message = ""};
      }
      case ReportQueryType::kRange: {
        const auto range =
            report_api_support::ParseRangeArgument(request.argument);
        return {.ok = true,
                .kind = StructuredReportKind::kRange,
                .report = report_data_query_service_->QueryRange(
                    range.start_date, range.end_date),
                .error_message = ""};
      }
      case ReportQueryType::kWeek:
        return {
            .ok = true,
            .kind = StructuredReportKind::kWeek,
            .report = report_data_query_service_->QueryWeekly(request.argument),
            .error_message = ""};
      case ReportQueryType::kYear:
        return {
            .ok = true,
            .kind = StructuredReportKind::kYear,
            .report = report_data_query_service_->QueryYearly(request.argument),
            .error_message = ""};
    }

    return report_api_support::BuildStructuredReportFailure(
        "RunStructuredReportQuery", "Unhandled report query type.");
  } catch (const tracer_core::common::ReportingContractError& error) {
    auto failure = report_api_support::BuildStructuredReportFailure(
        "RunStructuredReportQuery", error);
    tracer_core::common::ApplyReportingContract(failure, error);
    return failure;
  } catch (const std::exception& exception) {
    return report_api_support::BuildStructuredReportFailure(
        "RunStructuredReportQuery", exception);
  } catch (...) {
    return report_api_support::BuildStructuredReportFailure(
        "RunStructuredReportQuery");
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

auto ReportApi::RunReportTargetsQuery(const ReportTargetsRequest& request)
    -> ReportTargetsOutput {
  try {
    if (!report_data_query_service_) {
      return {
          .ok = false,
          .type = request.type,
          .items = {},
          .error_message = core_api_failure::BuildErrorMessage(
              "RunReportTargetsQuery",
              "Report data query service is not configured."),
      };
    }

    switch (request.type) {
      case ReportTargetType::kDay:
        return {.ok = true,
                .type = request.type,
                .items = report_data_query_service_->ListDailyTargets(),
                .error_message = ""};
      case ReportTargetType::kMonth:
        return {.ok = true,
                .type = request.type,
                .items = report_data_query_service_->ListMonthlyTargets(),
                .error_message = ""};
      case ReportTargetType::kWeek:
        return {.ok = true,
                .type = request.type,
                .items = report_data_query_service_->ListWeeklyTargets(),
                .error_message = ""};
      case ReportTargetType::kYear:
        return {.ok = true,
                .type = request.type,
                .items = report_data_query_service_->ListYearlyTargets(),
                .error_message = ""};
    }
  } catch (const std::exception& exception) {
    return {.ok = false,
            .type = request.type,
            .items = {},
            .error_message = core_api_failure::BuildErrorMessage(
                "RunReportTargetsQuery", exception.what())};
  } catch (...) {
    return {.ok = false,
            .type = request.type,
            .items = {},
            .error_message =
                core_api_failure::BuildErrorMessage("RunReportTargetsQuery",
                                                    "Unknown error.")};
  }
}

}  // namespace tracer::core::application::use_cases
