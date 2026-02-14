// application/use_cases/time_tracer_core_api_report.cpp
#include <exception>
#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "application/interfaces/i_report_handler.hpp"
#include "application/ports/i_report_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/use_cases/time_tracer_core_api.hpp"
#include "application/use_cases/time_tracer_core_api_helpers.hpp"

using namespace time_tracer::core::dto;
namespace core_api_helpers =
    time_tracer::application::use_cases::core_api_helpers;

namespace {

constexpr int kPeriodSeparatorLength = 40;

}  // namespace

auto TimeTracerCoreApi::RunReportQuery(const ReportQueryRequest& request)
    -> TextOutput {
  try {
    if (kReport && report_dto_formatter_) {
      const auto kStructured = RunStructuredReportQuery(
          {.type = request.type, .argument = request.argument});
      if (!kStructured.ok) {
        if (!kStructured.error_message.empty()) {
          return {.ok = false,
                  .content = "",
                  .error_message = kStructured.error_message};
        }
        return core_api_helpers::BuildTextFailure(
            "RunReportQuery",
            "Structured report query failed without error message.");
      }
      return core_api_helpers::FormatStructuredReport(
          kStructured, request.format, *report_dto_formatter_);
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
        const int kDays = std::stoi(request.argument);
        return {
            .ok = true,
            .content = report_handler_.RunPeriodQuery(kDays, request.format),
            .error_message = ""};
      }
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
    return core_api_helpers::BuildTextFailure("RunReportQuery",
                                              "Unhandled report query type.");
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildTextFailure("RunReportQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildTextFailure("RunReportQuery");
  }
}

auto TimeTracerCoreApi::RunStructuredReportQuery(
    const StructuredReportQueryRequest& request) -> StructuredReportOutput {
  try {
    if (!kReport) {
      return core_api_helpers::BuildStructuredReportFailure(
          "RunStructuredReportQuery",
          "Report data query service is not configured.");
    }

    switch (request.type) {
      case ReportQueryType::kDay:
        return {.ok = true,
                .kind = StructuredReportKind::kDay,
                .report = kReport->QueryDaily(request.argument),
                .error_message = ""};
      case ReportQueryType::kMonth:
        return {.ok = true,
                .kind = StructuredReportKind::kMonth,
                .report = kReport->QueryMonthly(request.argument),
                .error_message = ""};
      case ReportQueryType::kRecent: {
        const int kDays = std::stoi(request.argument);
        return {.ok = true,
                .kind = StructuredReportKind::kRecent,
                .report = kReport->QueryPeriod(kDays),
                .error_message = ""};
      }
      case ReportQueryType::kWeek:
        return {.ok = true,
                .kind = StructuredReportKind::kWeek,
                .report = kReport->QueryWeekly(request.argument),
                .error_message = ""};
      case ReportQueryType::kYear:
        return {.ok = true,
                .kind = StructuredReportKind::kYear,
                .report = kReport->QueryYearly(request.argument),
                .error_message = ""};
    }
    return core_api_helpers::BuildStructuredReportFailure(
        "RunStructuredReportQuery", "Unhandled report query type.");
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildStructuredReportFailure(
        "RunStructuredReportQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildStructuredReportFailure(
        "RunStructuredReportQuery");
  }
}

auto TimeTracerCoreApi::RunPeriodBatchQuery(
    const PeriodBatchQueryRequest& request) -> TextOutput {
  try {
    if (kReport && report_dto_formatter_) {
      const auto kStructured =
          RunStructuredPeriodBatchQuery({.kDays = request.days_list});
      if (!kStructured.ok && kStructured.items.empty()) {
        if (!kStructured.error_message.empty()) {
          return {.ok = false,
                  .content = "",
                  .error_message = kStructured.error_message};
        }
        return core_api_helpers::BuildTextFailure(
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
          output << core_api_helpers::BuildPeriodBatchErrorLine(
              item.kDays, item.error_message);
          continue;
        }

        try {
          output << report_dto_formatter_->FormatPeriod(*item.report,
                                                        request.format);
        } catch (const std::exception& exception) {
          output << core_api_helpers::BuildPeriodBatchErrorLine(
              item.kDays, exception.what());
        } catch (...) {
          output << core_api_helpers::BuildPeriodBatchErrorLine(
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
    return core_api_helpers::BuildTextFailure("RunPeriodBatchQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildTextFailure("RunPeriodBatchQuery");
  }
}

auto TimeTracerCoreApi::RunStructuredPeriodBatchQuery(
    const StructuredPeriodBatchQueryRequest& request)
    -> StructuredPeriodBatchOutput {
  try {
    if (!kReport) {
      return core_api_helpers::BuildStructuredPeriodBatchFailure(
          "RunStructuredPeriodBatchQuery",
          "Report data query service is not configured.");
    }

    StructuredPeriodBatchOutput output = {
        .ok = true, .items = {}, .error_message = ""};
    output.items.reserve(request.kDays.size());

    for (const int kDays : request.kDays) {
      StructuredPeriodBatchItem item = {.kDays = kDays,
                                        .ok = true,
                                        .report = std::nullopt,
                                        .error_message = ""};
      try {
        item.report = kReport->QueryPeriod(kDays);
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
    return core_api_helpers::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildStructuredPeriodBatchFailure(
        "RunStructuredPeriodBatchQuery");
  }
}

auto TimeTracerCoreApi::RunReportExport(const ReportExportRequest& request)
    -> OperationAck {
  try {
    if (kReport && report_export_writer_) {
      switch (request.type) {
        case ReportExportType::kDay: {
          const auto kDailyReport = kReport->QueryDaily(request.argument);
          report_export_writer_->ExportSingleDay(request.argument, kDailyReport,
                                                 request.format);
          break;
        }
        case ReportExportType::kMonth: {
          const auto kMonthlyReport = kReport->QueryMonthly(request.argument);
          report_export_writer_->ExportSingleMonth(
              request.argument, kMonthlyReport, request.format);
          break;
        }
        case ReportExportType::kRecent: {
          const int kDays = std::stoi(request.argument);
          const auto kPeriodReport = kReport->QueryPeriod(kDays);
          report_export_writer_->ExportSinglePeriod(kDays, kPeriodReport,
                                                    request.format);
          break;
        }
        case ReportExportType::kWeek: {
          const auto kWeeklyReport = kReport->QueryWeekly(request.argument);
          report_export_writer_->ExportSingleWeek(
              request.argument, kWeeklyReport, request.format);
          break;
        }
        case ReportExportType::kYear: {
          const auto kYearlyReport = kReport->QueryYearly(request.argument);
          report_export_writer_->ExportSingleYear(
              request.argument, kYearlyReport, request.format);
          break;
        }
        case ReportExportType::kAllDay: {
          const auto kReports = kReport->QueryAllDaily();
          report_export_writer_->ExportAllDaily(kReports, request.format);
          break;
        }
        case ReportExportType::kAllMonth: {
          const auto kReports = kReport->QueryAllMonthly();
          report_export_writer_->ExportAllMonthly(kReports, request.format);
          break;
        }
        case ReportExportType::kAllRecent: {
          const auto kReports =
              kReport->QueryPeriodBatch(request.recent_days_list);
          report_export_writer_->ExportAllPeriod(kReports, request.format);
          break;
        }
        case ReportExportType::kAllWeek: {
          const auto kReports = kReport->QueryAllWeekly();
          report_export_writer_->ExportAllWeekly(kReports, request.format);
          break;
        }
        case ReportExportType::kAllYear: {
          const auto kReports = kReport->QueryAllYearly();
          report_export_writer_->ExportAllYearly(kReports, request.format);
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
        const int kDays = std::stoi(request.argument);
        report_handler_.RunExportSinglePeriodReport(kDays, request.format);
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
    return core_api_helpers::BuildOperationFailure("RunReportExport",
                                                   exception);
  } catch (...) {
    return core_api_helpers::BuildOperationFailure("RunReportExport");
  }
}
