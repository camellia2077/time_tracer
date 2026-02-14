// domain/logic/validator/common/validator_utils.cpp
#include "domain/logic/validator/common/validator_utils.hpp"

#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "domain/ports/diagnostics.hpp"

namespace validator {
namespace {

auto BuildLogHeader(const std::string& filename) -> std::string {
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

}  // namespace

static auto GetErrorTypeHeader(ErrorType type) -> std::string {
  switch (type) {
    case ErrorType::kSourceRemarkAfterEvent:
    case ErrorType::kSourceNoDateAtStart:
    case ErrorType::kSourceInvalidLineFormat:
    case ErrorType::kUnrecognizedActivity:
    case ErrorType::kSourceMissingYearHeader:
      return "Source file format errors (源文件格式错误):";
    case ErrorType::kIncorrectDayCountForMonth:
      return "Date Logic errors(日期逻辑错误):";
    case ErrorType::kDateContinuity:
      return "Date Continuity errors (日期中断/缺失):";
    case ErrorType::kTimeDiscontinuity:
      return "Activity Time Discontinuity errors (活动时间不连续):";
    case ErrorType::kMissingSleepNight:
      return "Lack of sleep activity errors(最后的活动项目缺少sleep活动):";
    case ErrorType::kJsonTooFewActivities:
      return "Activity count errors(活动数量错误):";
    // NOLINTNEXTLINE(bugprone-branch-clone)
    case ErrorType::kZeroDurationActivity:
      return "Activity duration errors(活动时长错误):";
    case ErrorType::kActivityDurationTooLong:
      return "Activity duration errors(活动时长错误):";
    case ErrorType::kFileAccess:
      return "File access errors:";
    case ErrorType::kStructural:
      return "Structural errors:";
    case ErrorType::kLineFormat:
      return "Line format errors:";
    case ErrorType::kLogical:
      return "Logical consistency errors:";
    default:
      return "Other errors:";
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void PrintGroupedErrors(const std::string& filename,
                        const std::set<Error>& errors) {
  time_tracer::domain::ports::EmitError("请根据以下错误信息，手动修正该文件。");
  std::map<ErrorType, std::vector<Error>> grouped_errors;
  for (const auto& err : errors) {
    grouped_errors[err.type].push_back(err);
  }

  std::ostringstream report_stream;
  report_stream << BuildLogHeader(filename);

  for (const auto& pair : grouped_errors) {
    std::string header = GetErrorTypeHeader(pair.first);
    time_tracer::domain::ports::EmitError("\n" + header);
    report_stream << header << "\n";
    for (const auto& err : pair.second) {
      std::string location_prefix;
      if (err.source_span.has_value() && err.source_span->HasLine()) {
        std::string file_label = err.source_span->file_path;
        if (file_label.empty()) {
          file_label = filename;
        }
        if (!file_label.empty()) {
          location_prefix = file_label + ":";
        }
        if (err.source_span->line_end > err.source_span->line_start &&
            err.source_span->line_start > 0) {
          location_prefix += std::to_string(err.source_span->line_start) + "-" +
                             std::to_string(err.source_span->line_end) +
                             ": Line " +
                             std::to_string(err.source_span->line_start) + "-" +
                             std::to_string(err.source_span->line_end) + ": ";
        } else {
          location_prefix += std::to_string(err.source_span->line_start) +
                             ": Line " +
                             std::to_string(err.source_span->line_start) + ": ";
        }
      } else if (err.line_number > 0) {
        location_prefix = "Line " + std::to_string(err.line_number) + ": ";
      }

      std::string error_message = location_prefix + err.message;
      time_tracer::domain::ports::EmitError("  " + error_message);
      report_stream << "  " << error_message << "\n";

      if (err.source_span.has_value() && !err.source_span->raw_text.empty()) {
        std::string raw_line = "    > " + err.source_span->raw_text;
        time_tracer::domain::ports::EmitError(raw_line);
        report_stream << raw_line << "\n";
      }
    }
  }
  EmitReportSavedMessage(FlushReport(report_stream));
}

static auto GetDiagnosticHeader(const std::string& code) -> std::string {
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

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void PrintDiagnostics(const std::string& fallback_filename,
                      const std::vector<Diagnostic>& diagnostics) {
  if (diagnostics.empty()) {
    return;
  }

  time_tracer::domain::ports::EmitError("请根据以下错误信息，手动修正该文件。");

  std::map<std::string, std::vector<Diagnostic>> grouped;
  for (const auto& diag : diagnostics) {
    std::string key = diag.code.empty() ? "unknown" : diag.code;
    grouped[key].push_back(diag);
  }

  std::ostringstream report_stream;
  report_stream << BuildLogHeader(fallback_filename);

  for (const auto& pair : grouped) {
    std::string header = GetDiagnosticHeader(pair.first);
    time_tracer::domain::ports::EmitError("\n" + header);
    report_stream << header << "\n";
    for (const auto& diag : pair.second) {
      std::string location_prefix;
      if (diag.source_span.has_value() && diag.source_span->HasLine()) {
        std::string file_label = diag.source_span->file_path;
        if (file_label.empty()) {
          file_label = fallback_filename;
        }
        if (!file_label.empty()) {
          location_prefix = file_label + ":";
        }
        if (diag.source_span->line_end > diag.source_span->line_start &&
            diag.source_span->line_start > 0) {
          location_prefix +=
              std::to_string(diag.source_span->line_start) + "-" +
              std::to_string(diag.source_span->line_end) + ": Line " +
              std::to_string(diag.source_span->line_start) + "-" +
              std::to_string(diag.source_span->line_end) + ": ";
        } else {
          location_prefix +=
              std::to_string(diag.source_span->line_start) + ": Line " +
              std::to_string(diag.source_span->line_start) + ": ";
        }
      } else if (!fallback_filename.empty()) {
        location_prefix = fallback_filename + ": ";
      }

      std::string error_message = location_prefix + diag.message;
      time_tracer::domain::ports::EmitError("  " + error_message);
      report_stream << "  " << error_message << "\n";

      if (diag.source_span.has_value() && !diag.source_span->raw_text.empty()) {
        std::string raw_line = "    > " + diag.source_span->raw_text;
        time_tracer::domain::ports::EmitError(raw_line);
        report_stream << raw_line << "\n";
      }
    }
  }

  EmitReportSavedMessage(FlushReport(report_stream));
}

}  // namespace validator
