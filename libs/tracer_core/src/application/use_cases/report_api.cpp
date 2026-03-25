#include "application/use_cases/report_api.hpp"

#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "application/use_cases/core_api_failure.hpp"
#include "application/use_cases/report_api_support.hpp"

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
                     ReportDtoFormatterPtr report_dto_formatter,
                     ReportExportWriterPtr report_export_writer)
    : report_handler_(report_handler),
      report_data_query_service_(std::move(report_data_query_service)),
      report_dto_formatter_(std::move(report_dto_formatter)),
      report_export_writer_(std::move(report_export_writer)) {}

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
                  .error_message = structured.error_message};
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
        const int days = std::stoi(request.argument);
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
        const int days = std::stoi(request.argument);
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

auto ReportApi::RunReportExport(const ReportExportRequest& request)
    -> OperationAck {
  try {
    if (report_data_query_service_ && report_export_writer_) {
      switch (request.type) {
        case ReportExportType::kDay: {
          const auto daily_report =
              report_data_query_service_->QueryDaily(request.argument);
          report_export_writer_->ExportSingleDay(request.argument, daily_report,
                                                 request.format);
          break;
        }
        case ReportExportType::kMonth: {
          const auto monthly_report =
              report_data_query_service_->QueryMonthly(request.argument);
          report_export_writer_->ExportSingleMonth(
              request.argument, monthly_report, request.format);
          break;
        }
        case ReportExportType::kRecent: {
          const int days = std::stoi(request.argument);
          const auto period_report =
              report_data_query_service_->QueryPeriod(days);
          report_export_writer_->ExportSinglePeriod(days, period_report,
                                                    request.format);
          break;
        }
        case ReportExportType::kWeek: {
          const auto weekly_report =
              report_data_query_service_->QueryWeekly(request.argument);
          report_export_writer_->ExportSingleWeek(
              request.argument, weekly_report, request.format);
          break;
        }
        case ReportExportType::kYear: {
          const auto yearly_report =
              report_data_query_service_->QueryYearly(request.argument);
          report_export_writer_->ExportSingleYear(
              request.argument, yearly_report, request.format);
          break;
        }
        case ReportExportType::kAllDay: {
          const auto reports = report_data_query_service_->QueryAllDaily();
          report_export_writer_->ExportAllDaily(reports, request.format);
          break;
        }
        case ReportExportType::kAllMonth: {
          const auto reports = report_data_query_service_->QueryAllMonthly();
          report_export_writer_->ExportAllMonthly(reports, request.format);
          break;
        }
        case ReportExportType::kAllRecent: {
          const auto reports = report_data_query_service_->QueryPeriodBatch(
              request.recent_days_list);
          report_export_writer_->ExportAllPeriod(reports, request.format);
          break;
        }
        case ReportExportType::kAllWeek: {
          const auto reports = report_data_query_service_->QueryAllWeekly();
          report_export_writer_->ExportAllWeekly(reports, request.format);
          break;
        }
        case ReportExportType::kAllYear: {
          const auto reports = report_data_query_service_->QueryAllYearly();
          report_export_writer_->ExportAllYearly(reports, request.format);
          break;
        }
      }
      return {.ok = true, .error_message = ""};
    }

    switch (request.type) {
      case ReportExportType::kDay:
        report_handler_.RunExportSingleDayReport(request.argument,
                                                 request.format);
        break;
      case ReportExportType::kMonth:
        report_handler_.RunExportSingleMonthReport(request.argument,
                                                   request.format);
        break;
      case ReportExportType::kRecent: {
        const int days = std::stoi(request.argument);
        report_handler_.RunExportSinglePeriodReport(days, request.format);
        break;
      }
      case ReportExportType::kWeek:
        report_handler_.RunExportSingleWeekReport(request.argument,
                                                  request.format);
        break;
      case ReportExportType::kYear:
        report_handler_.RunExportSingleYearReport(request.argument,
                                                  request.format);
        break;
      case ReportExportType::kAllDay:
        report_handler_.RunExportAllDailyReportsQuery(request.format);
        break;
      case ReportExportType::kAllMonth:
        report_handler_.RunExportAllMonthlyReportsQuery(request.format);
        break;
      case ReportExportType::kAllRecent:
        report_handler_.RunExportAllPeriodReportsQuery(request.recent_days_list,
                                                       request.format);
        break;
      case ReportExportType::kAllWeek:
        report_handler_.RunExportAllWeeklyReportsQuery(request.format);
        break;
      case ReportExportType::kAllYear:
        report_handler_.RunExportAllYearlyReportsQuery(request.format);
        break;
    }

    return {.ok = true, .error_message = ""};
  } catch (const std::exception& exception) {
    return core_api_failure::BuildOperationFailure("RunReportExport",
                                                   exception);
  } catch (...) {
    return core_api_failure::BuildOperationFailure("RunReportExport");
  }
}

}  // namespace tracer::core::application::use_cases
