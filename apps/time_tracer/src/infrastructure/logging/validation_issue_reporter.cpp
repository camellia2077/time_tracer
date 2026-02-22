// infrastructure/logging/validation_issue_reporter.cpp
#include "infrastructure/logging/validation_issue_reporter.hpp"

#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "domain/ports/diagnostics.hpp"

namespace infrastructure::logging {
namespace {

auto BuildLogHeader(std::string_view filename) -> std::string {
  std::ostringstream stream;
  stream
      << "\n文件 " << filename
      << " 的检验错误\n--------------------------------------------------\n\n";
  return stream.str();
}

auto FlushReport(const std::ostringstream& report_stream) -> bool {
  return time_tracer::domain::ports::AppendErrorReport(report_stream.str());
}

void EmitReportSavedMessage(bool appended) {
  const std::string kDestinationLabel =
      time_tracer::domain::ports::GetErrorReportDestinationLabel();
  if (appended) {
    time_tracer::domain::ports::EmitInfo("详细的错误日志已保存至: " +
                                         kDestinationLabel);
    return;
  }
  time_tracer::domain::ports::EmitWarn("详细错误日志写入失败，目标: " +
                                       kDestinationLabel);
}

auto GetErrorTypeHeader(validator::ErrorType type) -> std::string {
  switch (type) {
    case validator::ErrorType::kSourceRemarkAfterEvent:
    case validator::ErrorType::kSourceNoDateAtStart:
    case validator::ErrorType::kSourceInvalidLineFormat:
    case validator::ErrorType::kUnrecognizedActivity:
    case validator::ErrorType::kSourceMissingYearHeader:
      return "Source file format errors (源文件格式错误):";
    case validator::ErrorType::kIncorrectDayCountForMonth:
      return "Date Logic errors(日期逻辑错误):";
    case validator::ErrorType::kDateContinuity:
      return "Date Continuity errors (日期中断/缺失):";
    case validator::ErrorType::kTimeDiscontinuity:
      return "Activity Time Discontinuity errors (活动时间不连续):";
    case validator::ErrorType::kMissingSleepNight:
      return "Lack of sleep activity errors(最后的活动项目缺少sleep活动):";
    case validator::ErrorType::kJsonTooFewActivities:
      return "Activity count errors(活动数量错误):";
    case validator::ErrorType::kZeroDurationActivity:
    case validator::ErrorType::kActivityDurationTooLong:
      return "Activity duration errors(活动时长错误):";
    case validator::ErrorType::kFileAccess:
      return "File access errors:";
    case validator::ErrorType::kStructural:
      return "Structural errors:";
    case validator::ErrorType::kLineFormat:
      return "Line format errors:";
    case validator::ErrorType::kLogical:
      return "Logical consistency errors:";
    default:
      return "Other errors:";
  }
}

auto GetDiagnosticHeader(const std::string& code) -> std::string {
  if (code == "activity.duration.zero" ||
      code == "activity.duration.too_long") {
    return "Activity duration errors(活动时长错误):";
  }
  if (code == "activity.count.too_few") {
    return "Activity count errors(活动数量错误):";
  }
  if (code == "date.continuity.missing") {
    return "Date Continuity errors (日期中断/缺失):";
  }
  return "Other errors:";
}

auto BuildLocationPrefix(const validator::Error& error,
                         std::string_view fallback_label) -> std::string {
  if (error.source_span.has_value() && error.source_span->HasLine()) {
    std::string file_label = error.source_span->file_path;
    if (file_label.empty()) {
      file_label = std::string(fallback_label);
    }
    std::string location_prefix;
    if (!file_label.empty()) {
      location_prefix = file_label + ":";
    }
    if (error.source_span->line_end > error.source_span->line_start &&
        error.source_span->line_start > 0) {
      location_prefix += std::to_string(error.source_span->line_start) + "-" +
                         std::to_string(error.source_span->line_end) +
                         ": Line " +
                         std::to_string(error.source_span->line_start) + "-" +
                         std::to_string(error.source_span->line_end) + ": ";
    } else {
      location_prefix += std::to_string(error.source_span->line_start) +
                         ": Line " +
                         std::to_string(error.source_span->line_start) + ": ";
    }
    return location_prefix;
  }
  if (error.line_number > 0) {
    return "Line " + std::to_string(error.line_number) + ": ";
  }
  return "";
}

auto BuildLocationPrefix(const validator::Diagnostic& diagnostic,
                         std::string_view fallback_label) -> std::string {
  if (diagnostic.source_span.has_value() && diagnostic.source_span->HasLine()) {
    std::string file_label = diagnostic.source_span->file_path;
    if (file_label.empty()) {
      file_label = std::string(fallback_label);
    }
    std::string location_prefix;
    if (!file_label.empty()) {
      location_prefix = file_label + ":";
    }
    if (diagnostic.source_span->line_end > diagnostic.source_span->line_start &&
        diagnostic.source_span->line_start > 0) {
      location_prefix +=
          std::to_string(diagnostic.source_span->line_start) + "-" +
          std::to_string(diagnostic.source_span->line_end) + ": Line " +
          std::to_string(diagnostic.source_span->line_start) + "-" +
          std::to_string(diagnostic.source_span->line_end) + ": ";
    } else {
      location_prefix +=
          std::to_string(diagnostic.source_span->line_start) + ": Line " +
          std::to_string(diagnostic.source_span->line_start) + ": ";
    }
    return location_prefix;
  }
  if (!fallback_label.empty()) {
    return std::string(fallback_label) + ": ";
  }
  return "";
}

}  // namespace

void ValidationIssueReporter::ReportStructureErrors(
    std::string_view display_label, const std::set<validator::Error>& errors) {
  if (errors.empty()) {
    return;
  }

  time_tracer::domain::ports::EmitError("请根据以下错误信息，手动修正该文件。");

  std::map<validator::ErrorType, std::vector<validator::Error>> grouped_errors;
  for (const auto& error : errors) {
    grouped_errors[error.type].push_back(error);
  }

  std::ostringstream report_stream;
  report_stream << BuildLogHeader(display_label);

  for (const auto& [error_type, grouped] : grouped_errors) {
    const std::string kHeader = GetErrorTypeHeader(error_type);
    time_tracer::domain::ports::EmitError("\n" + kHeader);
    report_stream << kHeader << "\n";
    for (const auto& error : grouped) {
      const std::string kErrorMessage =
          BuildLocationPrefix(error, display_label) + error.message;
      time_tracer::domain::ports::EmitError("  " + kErrorMessage);
      report_stream << "  " << kErrorMessage << "\n";

      if (error.source_span.has_value() &&
          !error.source_span->raw_text.empty()) {
        const std::string kRawLine = "    > " + error.source_span->raw_text;
        time_tracer::domain::ports::EmitError(kRawLine);
        report_stream << kRawLine << "\n";
      }
    }
  }

  EmitReportSavedMessage(FlushReport(report_stream));
}

void ValidationIssueReporter::ReportLogicDiagnostics(
    std::string_view fallback_label,
    const std::vector<validator::Diagnostic>& diagnostics) {
  if (diagnostics.empty()) {
    return;
  }

  time_tracer::domain::ports::EmitError("请根据以下错误信息，手动修正该文件。");

  std::map<std::string, std::vector<validator::Diagnostic>> grouped;
  for (const auto& diagnostic : diagnostics) {
    const std::string kCodeKey =
        diagnostic.code.empty() ? "unknown" : diagnostic.code;
    grouped[kCodeKey].push_back(diagnostic);
  }

  std::ostringstream report_stream;
  report_stream << BuildLogHeader(fallback_label);

  for (const auto& [code, grouped_diagnostics] : grouped) {
    const std::string kHeader = GetDiagnosticHeader(code);
    time_tracer::domain::ports::EmitError("\n" + kHeader);
    report_stream << kHeader << "\n";
    for (const auto& diagnostic : grouped_diagnostics) {
      const std::string kErrorMessage =
          BuildLocationPrefix(diagnostic, fallback_label) + diagnostic.message;
      time_tracer::domain::ports::EmitError("  " + kErrorMessage);
      report_stream << "  " << kErrorMessage << "\n";

      if (diagnostic.source_span.has_value() &&
          !diagnostic.source_span->raw_text.empty()) {
        const std::string kRawLine =
            "    > " + diagnostic.source_span->raw_text;
        time_tracer::domain::ports::EmitError(kRawLine);
        report_stream << kRawLine << "\n";
      }
    }
  }

  EmitReportSavedMessage(FlushReport(report_stream));
}

}  // namespace infrastructure::logging
