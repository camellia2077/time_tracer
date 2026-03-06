// infrastructure/tests/validation_issue_reporter_tests.cpp
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/logging/validation_issue_reporter.hpp"
#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

constexpr int kUnrecognizedActivityLine = 14;
constexpr int kDurationDiagnosticLine = 81;

class CapturingDiagnosticsSink final
    : public tracer_core::domain::ports::IDiagnosticsSink {
 public:
  auto Emit(tracer_core::domain::ports::DiagnosticSeverity severity,
            std::string_view message) -> void override {
    if (severity == tracer_core::domain::ports::DiagnosticSeverity::kError) {
      errors_.append(message);
      errors_.push_back('\n');
      return;
    }

    infos_.append(message);
    infos_.push_back('\n');
  }

  [[nodiscard]] auto Errors() const -> const std::string& { return errors_; }
  [[nodiscard]] auto Infos() const -> const std::string& { return infos_; }

 private:
  std::string errors_;
  std::string infos_;
};

class CapturingErrorReportWriter final
    : public tracer_core::domain::ports::IErrorReportWriter {
 public:
  auto Append(std::string_view report_content) -> bool override {
    appended_.append(report_content);
    return true;
  }

  [[nodiscard]] auto DestinationLabel() const -> std::string override {
    return "captured-report.log";
  }

  [[nodiscard]] auto Appended() const -> const std::string& {
    return appended_;
  }

 private:
  std::string appended_;
};

class DiagnosticsStateGuard {
 public:
  DiagnosticsStateGuard()
      : previous_sink_(tracer_core::domain::ports::GetDiagnosticsSink()),
        previous_writer_(tracer_core::domain::ports::GetErrorReportWriter()) {}

  ~DiagnosticsStateGuard() {
    tracer_core::domain::ports::SetDiagnosticsSink(previous_sink_);
    tracer_core::domain::ports::SetErrorReportWriter(previous_writer_);
  }

 private:
  std::shared_ptr<tracer_core::domain::ports::IDiagnosticsSink> previous_sink_;
  std::shared_ptr<tracer_core::domain::ports::IErrorReportWriter>
      previous_writer_;
};

auto BuildErrorWithSpan(std::string file_path, int line, std::string raw_text)
    -> validator::Error {
  return validator::Error{
      .line_number = line,
      .message =
          "Unrecognized activity 'clang'. Please check spelling or "
          "update config file.",
      .type = validator::ErrorType::kUnrecognizedActivity,
      .source_span = SourceSpan{
          .file_path = std::move(file_path),
          .line_start = line,
          .line_end = line,
          .column_start = 1,
          .column_end = static_cast<int>(raw_text.size()),
          .raw_text = std::move(raw_text),
      }};
}

auto BuildDiagnosticWithSpan(std::string file_path, int line,
                             std::string raw_text) -> validator::Diagnostic {
  return validator::Diagnostic{
      .severity = validator::DiagnosticSeverity::kError,
      .code = "activity.duration.too_long",
      .message =
          "In file for date 2025-02-01: Activity duration exceeds 16 "
          "hours.",
      .source_span = SourceSpan{
          .file_path = std::move(file_path),
          .line_start = line,
          .line_end = line,
          .column_start = 1,
          .column_end = static_cast<int>(raw_text.size()),
          .raw_text = std::move(raw_text),
      }};
}

auto InstallCapturingPorts()
    -> std::pair<std::shared_ptr<CapturingDiagnosticsSink>,
                 std::shared_ptr<CapturingErrorReportWriter>> {
  auto sink = std::make_shared<CapturingDiagnosticsSink>();
  auto writer = std::make_shared<CapturingErrorReportWriter>();
  tracer_core::domain::ports::SetDiagnosticsSink(sink);
  tracer_core::domain::ports::SetErrorReportWriter(writer);
  return {sink, writer};
}

auto TestStructureReporterRendersPathAndLine(int& failures) -> void {
  DiagnosticsStateGuard guard;
  auto [sink, writer] = InstallCapturingPorts();
  infrastructure::logging::ValidationIssueReporter reporter;

  std::set<validator::Error> errors;
  errors.insert(
      BuildErrorWithSpan(R"(C:\test\invalid\unrecognized_activity_sample.txt)",
                         kUnrecognizedActivityLine, "0940clang//clang tidy"));
  reporter.ReportStructureErrors("unrecognized_activity_sample.txt", errors);

  if (!Contains(sink->Errors(),
                "C:\\test\\invalid\\unrecognized_activity_sample.txt:14: "
                "Line 14: Unrecognized activity 'clang'.")) {
    ++failures;
    std::cerr << "[FAIL] Structure reporter should render clickable "
                 "path:line for unrecognized activity.\n";
  }

  if (!Contains(sink->Errors(), "> 0940clang//clang tidy")) {
    ++failures;
    std::cerr
        << "[FAIL] Structure reporter should render offending raw line.\n";
  }

  if (!Contains(writer->Appended(),
                "C:\\test\\invalid\\unrecognized_activity_sample.txt:14: "
                "Line 14: Unrecognized activity 'clang'.")) {
    ++failures;
    std::cerr << "[FAIL] Structure reporter should persist path:line into "
                 "error report writer.\n";
  }
}

auto TestLogicReporterPrefersSourceSpanPath(int& failures) -> void {
  DiagnosticsStateGuard guard;
  auto [sink, writer] = InstallCapturingPorts();
  infrastructure::logging::ValidationIssueReporter reporter;

  std::vector<validator::Diagnostic> diagnostics;
  diagnostics.push_back(BuildDiagnosticWithSpan(
      R"(C:\test\dates\2025-02.txt)", kDurationDiagnosticLine, "2016sleep"));
  reporter.ReportLogicDiagnostics("ProcessedData[2025-02]", diagnostics);

  if (!Contains(sink->Errors(),
                "C:\\test\\dates\\2025-02.txt:81: Line 81: In file for date "
                "2025-02-01: Activity duration exceeds 16 hours.")) {
    ++failures;
    std::cerr << "[FAIL] Logic reporter should prefer source-span file path "
                 "over pseudo fallback label.\n";
  }

  if (Contains(sink->Errors(), "ProcessedData[2025-02]:")) {
    ++failures;
    std::cerr << "[FAIL] Logic reporter should not emit pseudo fallback label "
                 "when source-span path exists.\n";
  }

  if (!Contains(writer->Appended(), R"(C:\test\dates\2025-02.txt:81:)")) {
    ++failures;
    std::cerr << "[FAIL] Logic reporter should persist source path into "
                 "error report.\n";
  }
}

}  // namespace

auto RunValidationIssueReporterTests(int& failures) -> void {
  TestStructureReporterRendersPathAndLine(failures);
  TestLogicReporterPrefersSourceSpanPath(failures);
}

}  // namespace android_runtime_tests
