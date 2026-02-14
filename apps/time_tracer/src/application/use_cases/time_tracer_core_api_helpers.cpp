// application/use_cases/time_tracer_core_api_helpers.cpp
#include "application/use_cases/time_tracer_core_api_helpers.hpp"

#include <utility>

#include "application/ports/i_report_dto_formatter.hpp"

namespace time_tracer::application::use_cases::core_api_helpers {

using namespace time_tracer::core::dto;

auto BuildErrorMessage(std::string_view operation, std::string_view details)
    -> std::string {
  if (details.empty()) {
    return std::string(operation) + " failed.";
  }
  return std::string(operation) + " failed: " + std::string(details);
}

auto BuildOperationFailure(std::string_view operation,
                           const std::exception& exception) -> OperationAck {
  return {.ok = false,
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildOperationFailure(std::string_view operation) -> OperationAck {
  return {.ok = false,
          .error_message =
              BuildErrorMessage(operation, "Unknown non-standard exception.")};
}

auto BuildTextFailure(std::string_view operation,
                      const std::exception& exception) -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildTextFailure(std::string_view operation, std::string_view details)
    -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message = BuildErrorMessage(operation, details)};
}

auto BuildTextFailure(std::string_view operation) -> TextOutput {
  return {.ok = false,
          .content = "",
          .error_message =
              BuildErrorMessage(operation, "Unknown non-standard exception.")};
}

auto BuildTreeFailure(std::string_view operation,
                      const std::exception& exception) -> TreeQueryResponse {
  return {.ok = false,
          .found = false,
          .roots = {},
          .nodes = {},
          .error_message = BuildErrorMessage(operation, exception.what())};
}

auto BuildTreeFailure(std::string_view operation) -> TreeQueryResponse {
  return {
      .ok = false,
      .found = false,
      .roots = {},
      .nodes = {},
      .error_message =
          BuildErrorMessage(operation, "Unknown non-standard exception."),
  };
}

auto BuildStructuredReportFailure(std::string_view operation,
                                  std::string_view details)
    -> StructuredReportOutput {
  return {.ok = false,
          .kind = StructuredReportKind::kDay,
          .report = DailyReportData{},
          .error_message = BuildErrorMessage(operation, details)};
}

auto BuildStructuredReportFailure(std::string_view operation,
                                  const std::exception& exception)
    -> StructuredReportOutput {
  return BuildStructuredReportFailure(operation, exception.what());
}

auto BuildStructuredReportFailure(std::string_view operation)
    -> StructuredReportOutput {
  return BuildStructuredReportFailure(operation,
                                      "Unknown non-standard exception.");
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       std::string_view details)
    -> StructuredPeriodBatchOutput {
  return {.ok = false,
          .items = {},
          .error_message = BuildErrorMessage(operation, details)};
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation,
                                       const std::exception& exception)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation, exception.what());
}

auto BuildStructuredPeriodBatchFailure(std::string_view operation)
    -> StructuredPeriodBatchOutput {
  return BuildStructuredPeriodBatchFailure(operation,
                                           "Unknown non-standard exception.");
}

auto FormatStructuredReport(
    const StructuredReportOutput& output, ReportFormat format,
    time_tracer::application::ports::IReportDtoFormatter& formatter)
    -> TextOutput {
  switch (output.kind) {
    case StructuredReportKind::kDay: {
      const auto* report = std::get_if<DailyReportData>(&output.report);
      if (report == nullptr) {
        return BuildTextFailure("RunReportQuery",
                                "Structured report kind/data mismatch: day.");
      }
      return {.ok = true,
              .content = formatter.FormatDaily(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kMonth: {
      const auto* report = std::get_if<MonthlyReportData>(&output.report);
      if (report == nullptr) {
        return BuildTextFailure("RunReportQuery",
                                "Structured report kind/data mismatch: month.");
      }
      return {.ok = true,
              .content = formatter.FormatMonthly(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kRecent: {
      const auto* report = std::get_if<PeriodReportData>(&output.report);
      if (report == nullptr) {
        return BuildTextFailure(
            "RunReportQuery",
            "Structured report kind/data mismatch: recent period.");
      }
      return {.ok = true,
              .content = formatter.FormatPeriod(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kWeek: {
      const auto* report = std::get_if<WeeklyReportData>(&output.report);
      if (report == nullptr) {
        return BuildTextFailure("RunReportQuery",
                                "Structured report kind/data mismatch: week.");
      }
      return {.ok = true,
              .content = formatter.FormatWeekly(*report, format),
              .error_message = ""};
    }
    case StructuredReportKind::kYear: {
      const auto* report = std::get_if<YearlyReportData>(&output.report);
      if (report == nullptr) {
        return BuildTextFailure("RunReportQuery",
                                "Structured report kind/data mismatch: year.");
      }
      return {.ok = true,
              .content = formatter.FormatYearly(*report, format),
              .error_message = ""};
    }
  }

  return BuildTextFailure("RunReportQuery",
                          "Unhandled structured report query kind.");
}

auto BuildPeriodBatchErrorLine(int days, std::string_view details)
    -> std::string {
  if (details.empty()) {
    return "Error querying period " + std::to_string(days) + " days.";
  }
  return "Error querying period " + std::to_string(days) +
         " days: " + std::string(details);
}

}  // namespace time_tracer::application::use_cases::core_api_helpers
