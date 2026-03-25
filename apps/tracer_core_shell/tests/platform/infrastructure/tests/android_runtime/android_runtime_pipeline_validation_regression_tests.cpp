#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

namespace modports = tracer_core::domain::ports;
namespace applog = tracer_core::application::runtime_bridge;

class CapturingLogger final : public applog::ILogger {
 public:
  auto Log(applog::LogSeverity /*severity*/, std::string_view message)
      -> void override {
    lines_.emplace_back(message);
  }

  [[nodiscard]] auto Joined() const -> std::string {
    std::string output;
    for (const auto& line : lines_) {
      output.append(line);
      output.push_back('\n');
    }
    return output;
  }

 private:
  std::vector<std::string> lines_;
};

class CapturingDiagnosticsSink final : public modports::IDiagnosticsSink {
 public:
  auto Emit(modports::DiagnosticSeverity severity,
            std::string_view message) -> void override {
    if (severity == modports::DiagnosticSeverity::kError) {
      errors_.append(message);
      errors_.push_back('\n');
    }
  }

  [[nodiscard]] auto Errors() const -> const std::string& { return errors_; }

 private:
  std::string errors_;
};

class CapturingErrorReportWriter final : public modports::IErrorReportWriter {
 public:
  auto Append(std::string_view report_content) -> bool override {
    appended_.append(report_content);
    return true;
  }

  [[nodiscard]] auto DestinationLabel() const -> std::string override {
    return "captured-validate-logic-report.log";
  }

  [[nodiscard]] auto Appended() const -> const std::string& {
    return appended_;
  }

 private:
  std::string appended_;
};

auto TestValidateLogicRejectsWakeKeywordAfterFirstEvent(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_logic_wake_position_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  auto logger = std::make_shared<CapturingLogger>();
  auto diagnostics_sink = std::make_shared<CapturingDiagnosticsSink>();
  auto error_report_writer = std::make_shared<CapturingErrorReportWriter>();

  infrastructure::bootstrap::AndroidRuntimeRequest request =
      BuildRuntimeRequest(kPaths, kConfigTomlPath);
  request.logger = logger;
  request.diagnostics_sink = diagnostics_sink;
  request.error_report_writer = error_report_writer;

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for wake-position "
                 "logic validation test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot = kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(
          kSourceFile,
          "y2026\nm03\n0301\n0700w\n0800高等数学\n0900wake\n1000有氧训练\n")) {
    ++failures;
    std::cerr << "[FAIL] Wake-position logic validation test should write "
                 "input file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunValidateLogic(
      {.input_path = kSourceRoot.string(),
       .date_check_mode = DateCheckMode::kNone});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should fail when wake keyword "
                 "appears after the first event.\n";
    cleanup();
    return;
  }

  const std::string kExpectedText =
      "Wake keyword activity 'wake' must appear only as the first event of the day.";
  if (!Contains(diagnostics_sink->Errors(), kExpectedText) ||
      !Contains(diagnostics_sink->Errors(), kSourceFile.string() + ":6")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should report clickable wake-order "
                 "diagnostic with source line.\n";
  }

  if (!Contains(kAck.error_message, "Recent diagnostics:") ||
      !Contains(kAck.error_message, kExpectedText)) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should surface wake-order logic "
                 "diagnostics in top-level error_message.\n";
  }

  cleanup();
}

auto TestConvertLogsActualConversionFailure(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_convert_failure_wording_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  auto logger = std::make_shared<CapturingLogger>();
  auto diagnostics_sink = std::make_shared<CapturingDiagnosticsSink>();
  auto error_report_writer = std::make_shared<CapturingErrorReportWriter>();

  infrastructure::bootstrap::AndroidRuntimeRequest request =
      BuildRuntimeRequest(kPaths, kConfigTomlPath);
  request.logger = logger;
  request.diagnostics_sink = diagnostics_sink;
  request.error_report_writer = error_report_writer;

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for conversion "
                 "failure wording test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot = kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\n0101\nr\n")) {
    ++failures;
    std::cerr << "[FAIL] Conversion failure wording test should write input "
                 "file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunConvert(
      {.input_path = kSourceRoot.string(),
       .date_check_mode = DateCheckMode::kNone,
       .save_processed_output = false,
       .validate_logic = false,
       .validate_structure = false});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should fail for malformed source line "
                 "without structure precheck.\n";
    cleanup();
    return;
  }

  const std::string kLogs = logger->Joined();
  if (!Contains(kLogs, "Step: Converting files (Parallel)...")) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should announce direct conversion when no "
                 "precheck is enabled.\n";
  }
  if (!Contains(kLogs, "[Pipeline] 转换阶段失败，流程终止。")) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should report actual conversion-stage "
                 "failure with dedicated wording.\n";
  }
  if (Contains(kLogs, "转换前结构预检失败")) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should not report structure precheck "
                 "failure wording when conversion actually failed.\n";
  }

  const std::string kExpectedParseError =
      kSourceFile.string() + ":4: Parse error: Invalid event line format => 'r'";
  if (!Contains(diagnostics_sink->Errors(), kExpectedParseError)) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should still surface parser-style "
                 "conversion errors when no precheck is enabled.\n";
  }

  if (!Contains(error_report_writer->Appended(), kExpectedParseError)) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should keep writing conversion failures "
                 "into the error report writer.\n";
  }

  if (!Contains(kAck.error_message,
                "Full error report: captured-validate-logic-report.log")) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should still advertise the saved error "
                 "report path.\n";
  }

  cleanup();
}

auto TestValidateStructureReportsInvalidUtf8(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_structure_invalid_utf8_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for invalid UTF-8 "
                 "validation test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot = kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  std::error_code error;
  std::filesystem::create_directories(kSourceRoot, error);
  if (error) {
    ++failures;
    std::cerr << "[FAIL] Invalid UTF-8 validation test should create source "
                 "directory.\n";
    cleanup();
    return;
  }
  {
    std::ofstream output(kSourceFile, std::ios::binary | std::ios::trunc);
    output.put(static_cast<char>(0xFF));
    output.put('\n');
    if (!output.good()) {
      ++failures;
      std::cerr << "[FAIL] Invalid UTF-8 validation test should write source "
                   "bytes.\n";
      cleanup();
      return;
    }
  }

  const auto kAck =
      runtime.runtime_api->pipeline().RunValidateStructure({.input_path = kSourceRoot.string()});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should fail for invalid UTF-8 "
                 "input.\n";
    cleanup();
    return;
  }

  if (!Contains(kAck.error_message, "Invalid UTF-8 in") ||
      !Contains(kAck.error_message, kSourceFile.string())) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should propagate invalid UTF-8 "
                 "details to error_message.\n";
  }
  if (Contains(kAck.error_message, "Validate structure pipeline failed")) {
    ++failures;
    std::cerr << "[FAIL] Invalid UTF-8 validation should not collapse to a "
                 "generic pipeline-failed message.\n";
  }

  cleanup();
}

}  // namespace

auto RunPipelineValidationRegressionTests(int& failures) -> void {
  TestValidateLogicRejectsWakeKeywordAfterFirstEvent(failures);
  TestConvertLogsActualConversionFailure(failures);
  TestValidateStructureReportsInvalidUtf8(failures);
}

}  // namespace android_runtime_tests
