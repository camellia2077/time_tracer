// application/use_cases/tracer_core_api_report.cpp
#include <cctype>
#include <chrono>
#include <exception>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "application/interfaces/i_report_handler.hpp"
#include "application/ports/i_report_data_query_service.hpp"
#include "application/ports/i_report_dto_formatter.hpp"
#include "application/ports/i_report_export_writer.hpp"
#include "application/use_cases/tracer_core_api.hpp"
#include "application/use_cases/tracer_core_api_helpers.hpp"

using namespace tracer_core::core::dto;
namespace core_api_helpers =
    tracer_core::application::use_cases::core_api_helpers;

namespace {

constexpr int kPeriodSeparatorLength = 40;
constexpr int kDecimalBase = 10;
constexpr size_t kIsoDateLength = 10U;
constexpr size_t kIsoYearLength = 4U;
constexpr size_t kIsoMonthSeparatorIndex = 7U;
constexpr size_t kIsoMonthStartIndex = 5U;
constexpr size_t kIsoDayStartIndex = 8U;
constexpr size_t kIsoPartLength = 2U;

struct DateRangeArgument {
  std::string start_date;
  std::string end_date;
};

[[nodiscard]] auto TrimAscii(std::string_view value) -> std::string_view {
  size_t begin = 0;
  while (begin < value.size() &&
         std::isspace(static_cast<unsigned char>(value[begin])) != 0) {
    ++begin;
  }

  size_t end = value.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
    --end;
  }

  return value.substr(begin, end - begin);
}

auto ParseUnsigned(std::string_view value, int& out_value) -> bool {
  if (value.empty()) {
    return false;
  }

  int parsed = 0;
  for (const char kCharacter : value) {
    if (std::isdigit(static_cast<unsigned char>(kCharacter)) == 0) {
      return false;
    }
    parsed = (parsed * kDecimalBase) + (kCharacter - '0');
  }
  out_value = parsed;
  return true;
}

auto ParseIsoDate(std::string_view value, std::chrono::year_month_day& out_ymd)
    -> bool {
  if (value.size() != kIsoDateLength || value[kIsoYearLength] != '-' ||
      value[kIsoMonthSeparatorIndex] != '-') {
    return false;
  }

  int year = 0;
  int month = 0;
  int day = 0;
  if (!ParseUnsigned(value.substr(0U, kIsoYearLength), year) ||
      !ParseUnsigned(value.substr(kIsoMonthStartIndex, kIsoPartLength),
                     month) ||
      !ParseUnsigned(value.substr(kIsoDayStartIndex, kIsoPartLength), day)) {
    return false;
  }

  const std::chrono::year_month_day kYmd(
      std::chrono::year(year), std::chrono::month(static_cast<unsigned>(month)),
      std::chrono::day(static_cast<unsigned>(day)));
  if (!kYmd.ok()) {
    return false;
  }

  out_ymd = kYmd;
  return true;
}

auto ParseRangeArgument(std::string_view argument) -> DateRangeArgument {
  const std::string_view kTrimmed = TrimAscii(argument);
  const size_t kSeparatorPos = kTrimmed.find('|');
  const size_t kCommaPos = kTrimmed.find(',');

  size_t split_pos = std::string_view::npos;
  if (kSeparatorPos != std::string_view::npos) {
    split_pos = kSeparatorPos;
  } else if (kCommaPos != std::string_view::npos) {
    split_pos = kCommaPos;
  }

  if (split_pos == std::string_view::npos) {
    throw std::invalid_argument(
        "Range argument must be `start|end` or `start,end` "
        "(ISO YYYY-MM-DD).");
  }

  const std::string_view kStartView = TrimAscii(kTrimmed.substr(0U, split_pos));
  const std::string_view kEndView = TrimAscii(kTrimmed.substr(split_pos + 1U));
  if (kStartView.empty() || kEndView.empty()) {
    throw std::invalid_argument("Range start/end date must not be empty.");
  }

  std::chrono::year_month_day start_ymd;
  std::chrono::year_month_day end_ymd;
  if (!ParseIsoDate(kStartView, start_ymd)) {
    throw std::invalid_argument("Invalid range start date (ISO YYYY-MM-DD): " +
                                std::string(kStartView));
  }
  if (!ParseIsoDate(kEndView, end_ymd)) {
    throw std::invalid_argument("Invalid range end date (ISO YYYY-MM-DD): " +
                                std::string(kEndView));
  }
  if (std::chrono::sys_days(start_ymd) > std::chrono::sys_days(end_ymd)) {
    throw std::invalid_argument("Range start date must be <= end date.");
  }

  return {.start_date = std::string(kStartView),
          .end_date = std::string(kEndView)};
}

}  // namespace

auto TracerCoreApi::RunReportQuery(const ReportQueryRequest& request)
    -> TextOutput {
  try {
    if (kReport_ && report_dto_formatter_) {
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
      case ReportQueryType::kRange:
        return core_api_helpers::BuildTextFailure(
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
    return core_api_helpers::BuildTextFailure("RunReportQuery",
                                              "Unhandled report query type.");
  } catch (const std::exception& exception) {
    return core_api_helpers::BuildTextFailure("RunReportQuery", exception);
  } catch (...) {
    return core_api_helpers::BuildTextFailure("RunReportQuery");
  }
}

auto TracerCoreApi::RunStructuredReportQuery(
    const StructuredReportQueryRequest& request) -> StructuredReportOutput {
  try {
    if (!kReport_) {
      return core_api_helpers::BuildStructuredReportFailure(
          "RunStructuredReportQuery",
          "Report data query service is not configured.");
    }

    switch (request.type) {
      case ReportQueryType::kDay:
        return {.ok = true,
                .kind = StructuredReportKind::kDay,
                .report = kReport_->QueryDaily(request.argument),
                .error_message = ""};
      case ReportQueryType::kMonth:
        return {.ok = true,
                .kind = StructuredReportKind::kMonth,
                .report = kReport_->QueryMonthly(request.argument),
                .error_message = ""};
      case ReportQueryType::kRecent: {
        const int kDays = std::stoi(request.argument);
        return {.ok = true,
                .kind = StructuredReportKind::kRecent,
                .report = kReport_->QueryPeriod(kDays),
                .error_message = ""};
      }
      case ReportQueryType::kRange: {
        const DateRangeArgument kRange = ParseRangeArgument(request.argument);
        return {
            .ok = true,
            .kind = StructuredReportKind::kRange,
            .report = kReport_->QueryRange(kRange.start_date, kRange.end_date),
            .error_message = ""};
      }
      case ReportQueryType::kWeek:
        return {.ok = true,
                .kind = StructuredReportKind::kWeek,
                .report = kReport_->QueryWeekly(request.argument),
                .error_message = ""};
      case ReportQueryType::kYear:
        return {.ok = true,
                .kind = StructuredReportKind::kYear,
                .report = kReport_->QueryYearly(request.argument),
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

auto TracerCoreApi::RunPeriodBatchQuery(const PeriodBatchQueryRequest& request)
    -> TextOutput {
  try {
    if (kReport_ && report_dto_formatter_) {
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

auto TracerCoreApi::RunStructuredPeriodBatchQuery(
    const StructuredPeriodBatchQueryRequest& request)
    -> StructuredPeriodBatchOutput {
  try {
    if (!kReport_) {
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
        item.report = kReport_->QueryPeriod(kDays);
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

auto TracerCoreApi::RunReportExport(const ReportExportRequest& request)
    -> OperationAck {
  try {
    if (kReport_ && report_export_writer_) {
      switch (request.type) {
        case ReportExportType::kDay: {
          const auto kDailyReport = kReport_->QueryDaily(request.argument);
          report_export_writer_->ExportSingleDay(request.argument, kDailyReport,
                                                 request.format);
          break;
        }
        case ReportExportType::kMonth: {
          const auto kMonthlyReport = kReport_->QueryMonthly(request.argument);
          report_export_writer_->ExportSingleMonth(
              request.argument, kMonthlyReport, request.format);
          break;
        }
        case ReportExportType::kRecent: {
          const int kDays = std::stoi(request.argument);
          const auto kPeriodReport = kReport_->QueryPeriod(kDays);
          report_export_writer_->ExportSinglePeriod(kDays, kPeriodReport,
                                                    request.format);
          break;
        }
        case ReportExportType::kWeek: {
          const auto kWeeklyReport = kReport_->QueryWeekly(request.argument);
          report_export_writer_->ExportSingleWeek(
              request.argument, kWeeklyReport, request.format);
          break;
        }
        case ReportExportType::kYear: {
          const auto kYearlyReport = kReport_->QueryYearly(request.argument);
          report_export_writer_->ExportSingleYear(
              request.argument, kYearlyReport, request.format);
          break;
        }
        case ReportExportType::kAllDay: {
          const auto kReports = kReport_->QueryAllDaily();
          report_export_writer_->ExportAllDaily(kReports, request.format);
          break;
        }
        case ReportExportType::kAllMonth: {
          const auto kReports = kReport_->QueryAllMonthly();
          report_export_writer_->ExportAllMonthly(kReports, request.format);
          break;
        }
        case ReportExportType::kAllRecent: {
          const auto kReports =
              kReport_->QueryPeriodBatch(request.recent_days_list);
          report_export_writer_->ExportAllPeriod(kReports, request.format);
          break;
        }
        case ReportExportType::kAllWeek: {
          const auto kReports = kReport_->QueryAllWeekly();
          report_export_writer_->ExportAllWeekly(kReports, request.format);
          break;
        }
        case ReportExportType::kAllYear: {
          const auto kReports = kReport_->QueryAllYearly();
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
