// infrastructure/tests/validation_issue_reporter_tests.cpp
import tracer.core.infrastructure.logging;
import tracer.core.domain.logic.validator.common.diagnostic;
import tracer.core.domain.logic.validator.common.validator_utils;
import tracer.core.domain.model.source_span;
import tracer.core.domain.ports.diagnostics;

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

namespace infra_logging = tracer::core::infrastructure::logging;
namespace modports = tracer::core::domain::modports;
namespace validator_common = tracer::core::domain::modlogic::validator_common;

using tracer::core::domain::modmodel::SourceSpan;

constexpr int kUnrecognizedActivityLine = 14;
constexpr int kDurationDiagnosticLine = 81;

class CapturingDiagnosticsSink final : public modports::IDiagnosticsSink {
 public:
  auto Emit(modports::DiagnosticSeverity severity, std::string_view message)
      -> void override {
    if (severity == modports::DiagnosticSeverity::kError) {
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

class CapturingErrorReportWriter final : public modports::IErrorReportWriter {
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
      : previous_sink_(modports::GetDiagnosticsSink()),
        previous_writer_(modports::GetErrorReportWriter()) {}

  ~DiagnosticsStateGuard() {
    modports::SetDiagnosticsSink(previous_sink_);
    modports::SetErrorReportWriter(previous_writer_);
  }

 private:
  std::shared_ptr<modports::IDiagnosticsSink> previous_sink_;
  std::shared_ptr<modports::IErrorReportWriter> previous_writer_;
};

auto BuildErrorWithSpan(std::string file_path, int line, std::string raw_text)
    -> validator_common::Error {
  return validator_common::Error{
      .line_number = line,
      .message =
          "Unrecognized activity 'clang'. Please check spelling or "
          "update config file.",
      .type = validator_common::ErrorType::kUnrecognizedActivity,
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
                             std::string raw_text)
    -> validator_common::Diagnostic {
  return validator_common::Diagnostic{
      .severity = validator_common::DiagnosticSeverity::kError,
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
  modports::SetDiagnosticsSink(sink);
  modports::SetErrorReportWriter(writer);
  return {sink, writer};
}

auto TestStructureReporterRendersPathAndLine(int& failures) -> void {
  DiagnosticsStateGuard guard;
  auto [sink, writer] = InstallCapturingPorts();
  infra_logging::ValidationIssueReporter reporter;

  std::set<validator_common::Error> errors;
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
  infra_logging::ValidationIssueReporter reporter;

  std::vector<validator_common::Diagnostic> diagnostics;
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

auto TestReporterSkipsSaveNoticeWhenErrorReportWriterDisabled(int& failures)
    -> void {
  DiagnosticsStateGuard guard;
  auto sink = std::make_shared<CapturingDiagnosticsSink>();
  modports::SetDiagnosticsSink(sink);
  modports::SetErrorReportWriter(nullptr);
  infra_logging::ValidationIssueReporter reporter;

  std::set<validator_common::Error> errors;
  errors.insert(
      BuildErrorWithSpan(R"(C:\test\invalid\terminal_only_sample.txt)",
                         kUnrecognizedActivityLine, "0940clang//clang tidy"));
  reporter.ReportStructureErrors("terminal_only_sample.txt", errors);

  if (!Contains(sink->Errors(),
                "C:\\test\\invalid\\terminal_only_sample.txt:14: "
                "Line 14: Unrecognized activity 'clang'.")) {
    ++failures;
    std::cerr << "[FAIL] Reporter should still emit validation diagnostics "
                 "when the error report writer is disabled.\n";
  }

  if (Contains(sink->Infos(), "详细的错误日志已保存至") ||
      Contains(sink->Infos(), "详细错误日志写入失败")) {
    ++failures;
    std::cerr << "[FAIL] Reporter should stay silent about report files when "
                 "the error report writer is disabled.\n";
  }
}

}  // namespace

auto RunValidationIssueReporterTests(int& failures) -> void {
  TestStructureReporterRendersPathAndLine(failures);
  TestLogicReporterPrefersSourceSpanPath(failures);
  TestReporterSkipsSaveNoticeWhenErrorReportWriterDisabled(failures);
}

}  // namespace android_runtime_tests
