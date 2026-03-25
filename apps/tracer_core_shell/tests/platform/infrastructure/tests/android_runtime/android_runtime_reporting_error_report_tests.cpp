#include <filesystem>
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
  auto Emit(modports::DiagnosticSeverity severity, std::string_view message)
      -> void override {
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

auto TestValidateLogicReusesStructureReporter(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_logic_structure_reporting_test");
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
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for validate-logic "
                 "structure reporting test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\n0101\nr\n")) {
    ++failures;
    std::cerr << "[FAIL] Validate-logic structure reporting test should write "
                 "input file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunValidateLogic(
      {.input_path = kSourceRoot.string(),
       .date_check_mode = DateCheckMode::kNone});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should fail for malformed source "
                 "line.\n";
    cleanup();
    return;
  }

  const std::string kExpectedLocation =
      kSourceFile.string() + ":4: Line 4: Unrecognized line format: r";
  if (!Contains(diagnostics_sink->Errors(), kExpectedLocation)) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should reuse structure reporter for "
                 "clickable path:line output.\n";
  }

  if (Contains(diagnostics_sink->Errors(), "Parse error:")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should not fall back to parser-style "
                 "parse error output when structure precheck is enabled.\n";
  }

  if (!error_report_writer->Appended().empty()) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should keep validate runs "
                 "terminal-only and skip the error report writer.\n";
  }

  if (Contains(kAck.error_message, "Full error report:")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should not advertise a saved error "
                 "report for validate runs.\n";
  }
  if (!Contains(kAck.error_message, "Recent diagnostics:") ||
      !Contains(kAck.error_message, "Unrecognized line format: r")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should surface buffered structure "
                 "diagnostics in the top-level error_message.\n";
  }

  const std::string kLogs = logger->Joined();
  if (!Contains(
          kLogs,
          "[STEP] Step: Pre-validating File Structure before Conversion...")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should announce structure precheck "
                 "before conversion.\n";
  }
  if (!Contains(kLogs, "[Pipeline] 转换前结构预检失败，已跳过转换阶段。")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should report precheck failure as "
                 "conversion-skipping pipeline stop.\n";
  }
  if (Contains(kLogs, "Step: Converting files after structure precheck")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should not enter conversion stage "
                 "when structure precheck fails.\n";
  }

  cleanup();
}

auto TestValidateStructureSkipsErrorReportFiles(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_structure_terminal_only_test");
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
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for validate-"
                 "structure terminal-only test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\n0101\nr\n")) {
    ++failures;
    std::cerr << "[FAIL] Validate-structure terminal-only test should write "
                 "input file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunValidateStructure(
      {.input_path = kSourceRoot.string()});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should fail for malformed "
                 "source line.\n";
    cleanup();
    return;
  }

  const std::string kExpectedLocation =
      kSourceFile.string() + ":4: Line 4: Unrecognized line format: r";
  if (!Contains(diagnostics_sink->Errors(), kExpectedLocation)) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should still emit clickable "
                 "terminal diagnostics.\n";
  }

  if (!error_report_writer->Appended().empty()) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should not write validation "
                 "errors into the error report writer.\n";
  }

  if (Contains(kAck.error_message, "Full error report:")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should not advertise a saved "
                 "error report.\n";
  }
  if (!Contains(kAck.error_message, "Recent diagnostics:") ||
      !Contains(kAck.error_message, "Unrecognized line format: r")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateStructure should surface buffered "
                 "diagnostics in the top-level error_message.\n";
  }

  cleanup();
}

auto TestAndroidDefaultRuntimeSkipsErrorReportFiles(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_default_error_report_disabled_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "assets" / "tracer_core" / "config" / "converter" /
      "interval_processor_config.toml";

  auto logger = std::make_shared<CapturingLogger>();
  auto diagnostics_sink = std::make_shared<CapturingDiagnosticsSink>();

  infrastructure::bootstrap::AndroidRuntimeRequest request =
      BuildRuntimeRequest(kPaths, kConfigTomlPath);
  request.logger = logger;
  request.diagnostics_sink = diagnostics_sink;

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for default "
                 "error-report policy test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\n0101\nr\n")) {
    ++failures;
    std::cerr << "[FAIL] Default error-report policy test should write input "
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
                 "when Android default writer is disabled.\n";
    cleanup();
    return;
  }

  const std::string kExpectedParseError =
      kSourceFile.string() +
      ":4: Parse error: Invalid event line format => 'r'";
  if (!Contains(diagnostics_sink->Errors(), kExpectedParseError)) {
    ++failures;
    std::cerr << "[FAIL] Android default runtime should still emit parser "
                 "errors to diagnostics sink.\n";
  }

  if (Contains(kAck.error_message, "Full error report:")) {
    ++failures;
    std::cerr << "[FAIL] Android default runtime should not advertise saved "
                 "error-report files.\n";
  }

  if (std::filesystem::exists(kPaths.output_root / "logs")) {
    ++failures;
    std::cerr << "[FAIL] Android default runtime should not create output/logs "
                 "for native error reports.\n";
  }

  cleanup();
}

}  // namespace

auto RunReportingErrorReportTests(int& failures) -> void {
  TestValidateLogicReusesStructureReporter(failures);
  TestValidateStructureSkipsErrorReportFiles(failures);
  TestAndroidDefaultRuntimeSkipsErrorReportFiles(failures);
}

}  // namespace android_runtime_tests
