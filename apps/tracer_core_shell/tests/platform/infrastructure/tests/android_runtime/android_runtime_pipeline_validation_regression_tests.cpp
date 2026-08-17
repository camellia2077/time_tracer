#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "infrastructure/tests/android_runtime/android_runtime_test_common.hpp"

namespace android_runtime_tests {
namespace {

namespace modports = tracer_core::domain::ports;
namespace applog = tracer_core::application::runtime_bridge;

constexpr std::string_view kIncompleteDayWarning =
    "Warning: this day currently has fewer than 2 authored events, so some "
    "intervals may not be computable yet.";
constexpr std::string_view kOvernightContinuationWarning =
    "Warning: possible overnight continuation; the first event of this day is "
    "not wake-related, so no sleep activity will be auto-generated.";

auto CopyFixtureFile(const std::filesystem::path& relative_path,
                     const std::filesystem::path& target_path) -> bool {
  std::error_code error;
  std::filesystem::create_directories(target_path.parent_path(), error);
  if (error) {
    return false;
  }

  std::filesystem::copy_file(BuildRepoRoot() / relative_path, target_path,
                             std::filesystem::copy_options::overwrite_existing,
                             error);
  return !error;
}

auto PrepareCustomConfigFixture(
    const std::filesystem::path& config_root,
    const std::filesystem::path& converter_main_fixture_relative_path)
    -> std::optional<std::filesystem::path> {
  if (!PrepareAndroidConfigFixture(config_root)) {
    return std::nullopt;
  }

  const std::filesystem::path user_root = config_root / "user";
  if (!CopyFixtureFile(converter_main_fixture_relative_path,
                       user_root / "behavior.toml")) {
    return std::nullopt;
  }
  return user_root / "behavior.toml";
}

auto BuildRecentWakeEventLine() -> std::string {
  const auto now = std::chrono::system_clock::now();
  const auto prior = now - std::chrono::minutes(1);
  const std::time_t prior_time = std::chrono::system_clock::to_time_t(prior);

  std::tm local_time{};
#ifdef _WIN32
  localtime_s(&local_time, &prior_time);
#else
  localtime_r(&prior_time, &local_time);
#endif

  std::ostringstream output;
  output << std::setw(2) << std::setfill('0') << local_time.tm_hour
         << std::setw(2) << std::setfill('0') << local_time.tm_min << "w\n";
  return output.str();
}

auto PrepareCustomWakeConfig(const std::filesystem::path& config_root)
    -> std::optional<std::filesystem::path> {
  return PrepareCustomConfigFixture(
      config_root, "test/fixtures/config/custom/sleep_inference.riseup.toml");
}

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
  auto Append(std::string_view insights_content) -> bool override {
    appended_.append(insights_content);
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
      kRepoRoot / "config" / "user" / "behavior.toml";

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

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(
          kSourceFile,
          "y2026\nm03\nd0301\n0700w\n0800高等数学\n0900wake\n1000有氧训练\n")) {
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
      "Wake keyword activity 'wake' must appear only as the first event of the "
      "day.";
  if (!Contains(diagnostics_sink->Errors(), kExpectedText) ||
      !Contains(diagnostics_sink->Errors(), kSourceFile.string() + ":6")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should insights clickable wake-order "
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

auto TestValidateLogicAllowsSingleAuthoredEventDay(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_logic_single_authored_event_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

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
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for single-day "
                 "logic validation test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\nd0301\n0700w\n")) {
    ++failures;
    std::cerr << "[FAIL] Single-authored-event logic validation test should "
                 "write input file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunValidateLogic(
      {.input_path = kSourceRoot.string(),
       .date_check_mode = DateCheckMode::kNone});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should allow a day with one "
                 "authored event: "
              << kAck.error_message << '\n';
    cleanup();
    return;
  }

  if (Contains(diagnostics_sink->Errors(), "less than 2 activities") ||
      Contains(error_report_writer->Appended(), "less than 2 activities")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should no longer emit the old "
                 "activity-count diagnostic for single-authored-event days.\n";
  }

  cleanup();
}

auto TestValidateLogicRejectsBadTimeRangeFixture(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_logic_bad_time_range_fixture_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  auto diagnostics_sink = std::make_shared<CapturingDiagnosticsSink>();
  auto error_report_writer = std::make_shared<CapturingErrorReportWriter>();

  infrastructure::bootstrap::AndroidRuntimeRequest request =
      BuildRuntimeRequest(kPaths, kConfigTomlPath);
  request.diagnostics_sink = diagnostics_sink;
  request.error_report_writer = error_report_writer;

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(request);
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for bad-time-range "
                 "fixture validation test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-01.txt";
  if (!CopyFixtureFile("test/fixtures/text/invalid/2026-01.bad_time_range.txt",
                       kSourceFile)) {
    ++failures;
    std::cerr << "[FAIL] Bad-time-range fixture validation test should copy "
                 "fixture input file.\n";
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunValidateLogic(
      {.input_path = kSourceRoot.string(),
       .date_check_mode = DateCheckMode::kNone});
  if (kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should fail for the bad-time-range "
                 "fixture.\n";
    cleanup();
    return;
  }

  if (!Contains(diagnostics_sink->Errors(), kSourceFile.string()) ||
      !Contains(diagnostics_sink->Errors(),
                "Activity duration must be positive")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should insights the bad-time-range "
                 "fixture as a positive-duration violation.\n";
  }

  if (!Contains(kAck.error_message, "Recent diagnostics:") ||
      !Contains(kAck.error_message, "Activity duration must be positive")) {
    ++failures;
    std::cerr << "[FAIL] RunValidateLogic should surface bad-time-range "
                 "fixture diagnostics in top-level error_message.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyAllowsFirstRecordOnLateMonthDay(int& failures)
    -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_first_late_month_day_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for the first "
                 "late-month record test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-31",
       .raw_activity_name = "w",
       .remark = "",
       .preferred_txt_path = "",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should allow the first "
                 "record on a late-month day: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  if (kAck.warnings.size() != 1U ||
      kAck.warnings.front() != std::string(kIncompleteDayWarning) ||
      !Contains(kAck.message, std::string(kIncompleteDayWarning)) ||
      Contains(kAck.message, std::string(kOvernightContinuationWarning))) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should surface only the "
                 "generic completeness warning for a wake-only new day.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyAcceptsWakeKeywordFromConfigOnly(int& failures)
    -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_warning_config_only_wake_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const auto kConfigTomlPath =
      PrepareCustomWakeConfig(kPaths.test_root / "config");
  if (!kConfigTomlPath.has_value()) {
    ++failures;
    std::cerr << "[FAIL] Config-only wake record warning test should prepare "
                 "custom config fixture.\n";
    cleanup();
    return;
  }

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, *kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for config-only "
                 "wake record warning test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "riseup",
       .remark = "",
       .preferred_txt_path = "",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should accept a wake "
                 "keyword that only exists in wake_keywords: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  if (kAck.warnings.size() != 1U ||
      kAck.warnings.front() != std::string(kIncompleteDayWarning) ||
      !Contains(kAck.message, std::string(kIncompleteDayWarning)) ||
      Contains(kAck.message, std::string(kOvernightContinuationWarning))) {
    ++failures;
    std::cerr << "[FAIL] Config-only wake token should produce only the "
                 "generic completeness warning.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyAcceptsCanonicalActivityToken(int& failures)
    -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_canonical_token_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for canonical "
                 "record token test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "recreation_game_clash-royale",
       .remark = "",
       .preferred_txt_path = "",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should accept canonical "
                 "activity tokens from alias mapping values: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kMonthFile =
      kPaths.test_root / "input" / "2026" / "2026-03.txt";
  std::ifstream month_input(kMonthFile);
  std::stringstream month_buffer;
  month_buffer << month_input.rdbuf();
  if (!month_input.good() ||
      !Contains(month_buffer.str(), "recreation_game_clash-royale")) {
    ++failures;
    std::cerr << "[FAIL] Canonical record token should be written into the "
                 "target TXT file unchanged.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyWarnsForOvernightContinuationDay(int& failures)
    -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_warning_continuation_day_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for continuation "
                 "record warning test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "哔哩哔哩",
       .remark = "",
       .preferred_txt_path = "",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should succeed for an "
                 "overnight-continuation new day: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  if (kAck.warnings.size() != 1U ||
      kAck.warnings.front() != std::string(kOvernightContinuationWarning) ||
      !Contains(kAck.message, std::string(kOvernightContinuationWarning)) ||
      Contains(kAck.message, std::string(kIncompleteDayWarning))) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should surface only the "
                 "overnight warning for an incomplete continuation day.\n";
  }

  const std::filesystem::path kMonthFile =
      kPaths.test_root / "input" / "2026" / "2026-03.txt";
  std::ifstream month_input(kMonthFile);
  std::stringstream month_buffer;
  month_buffer << month_input.rdbuf();
  if (!month_input.good() || !Contains(month_buffer.str(), "bilibili") ||
      Contains(month_buffer.str(), "哔哩哔哩")) {
    ++failures;
    std::cerr << "[FAIL] Record should write the canonical activity name to "
                 "the target TXT file when given an alias.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyPreservesMultilineRemark(int& failures)
    -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_multiline_remark_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };
  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for multiline "
                 "remark test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "bilibili",
       .remark = "first line\nsecond line\\path",
       .preferred_txt_path = "",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should preserve a "
                 "multiline remark: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  const std::filesystem::path kMonthFile =
      kPaths.test_root / "input" / "2026" / "2026-03.txt";
  std::ifstream month_input(kMonthFile);
  std::stringstream month_buffer;
  month_buffer << month_input.rdbuf();
  const std::string month_text = month_buffer.str();
  if (!month_input.good() ||
      !Contains(month_text, "// first line\n// second line\\path")) {
    ++failures;
    std::cerr << "[FAIL] Atomic record should write multiline remarks as "
                 "physical // continuation lines.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallyInsertsBeforeTrailingDaySeparator(
    int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_trailing_separator_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };
  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";
  const std::filesystem::path kMonthFile =
      kPaths.test_root / "input" / "2026" / "2026-03.txt";
  const std::string month_file_text =
      std::string("y2026\nm03\n\nd0301\n") + BuildRecentWakeEventLine() +
      "\nd0302\n0656w\n";
  if (!WriteFileWithParents(kMonthFile, month_file_text)) {
    ++failures;
    std::cerr << "[FAIL] Trailing-separator record test should write the "
                 "pre-existing month file.\n";
    cleanup();
    return;
  }

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for trailing "
                 "separator record test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "bilibili",
       .remark = "",
       .preferred_txt_path = "2026/2026-03.txt",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should insert before a "
                 "trailing day separator: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  std::ifstream month_input(kMonthFile);
  std::stringstream month_buffer;
  month_buffer << month_input.rdbuf();
  const std::string month_text = month_buffer.str();
  const std::size_t separator = month_text.find("\n\nd0302\n");
  const std::size_t day_start = month_text.find("d0301\n");
  if (!month_input.good() || separator == std::string::npos ||
      day_start == std::string::npos ||
      Contains(month_text.substr(day_start, separator - day_start), "\n\n")) {
    ++failures;
    std::cerr << "[FAIL] Atomic Record should insert before the blank day "
                 "separator left by Files activity deletion.\n";
  }

  cleanup();
}

auto TestRecordActivityAtomicallySkipsCompletenessWarningForCompleteDay(
    int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_record_warning_complete_day_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

  const std::filesystem::path kMonthFile =
      kPaths.test_root / "input" / "2026" / "2026-03.txt";
  const std::string month_file_text =
      std::string("y2026\nm03\nd0301\n") + BuildRecentWakeEventLine();
  if (!WriteFileWithParents(kMonthFile, month_file_text)) {
    ++failures;
    std::cerr << "[FAIL] Complete-day record warning test should write the "
                 "pre-existing month file.\n";
    cleanup();
    return;
  }

  infrastructure::bootstrap::AndroidRuntime runtime;
  try {
    runtime = infrastructure::bootstrap::BuildAndroidRuntime(
        BuildRuntimeRequest(kPaths, kConfigTomlPath));
  } catch (const std::exception& exception) {
    ++failures;
    std::cerr << "[FAIL] BuildAndroidRuntime should succeed for complete-day "
                 "record warning test: "
              << exception.what() << '\n';
    cleanup();
    return;
  }

  const auto kAck = runtime.runtime_api->pipeline().RunRecordActivityAtomically(
      {.target_date_iso = "2026-03-01",
       .raw_activity_name = "bilibili",
       .remark = "",
       .preferred_txt_path = "2026/2026-03.txt",
       .date_check_mode = DateCheckMode::kNone,
       .time_order_mode = TimeOrderMode::kStrictCalendar});
  if (!kAck.ok) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should succeed when the "
                 "target day becomes complete: "
              << kAck.message << '\n';
    cleanup();
    return;
  }

  if (!kAck.warnings.empty() ||
      Contains(kAck.message, std::string(kIncompleteDayWarning)) ||
      Contains(kAck.message, std::string(kOvernightContinuationWarning))) {
    ++failures;
    std::cerr << "[FAIL] RunRecordActivityAtomically should not emit "
                 "completeness warnings after the day has at least two "
                 "authored events.\n";
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
      kRepoRoot / "config" / "user" / "behavior.toml";

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

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
  const std::filesystem::path kSourceFile = kSourceRoot / "2026-03.txt";
  if (!WriteFileWithParents(kSourceFile, "y2026\nm03\nd0301\nr\n")) {
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
    std::cerr << "[FAIL] RunConvert should insights actual conversion-stage "
                 "failure with dedicated wording.\n";
  }
  if (Contains(kLogs, "转换前结构预检失败")) {
    ++failures;
    std::cerr << "[FAIL] RunConvert should not insights structure precheck "
                 "failure wording when conversion actually failed.\n";
  }

  const std::string kExpectedParseError =
      "Parse error: Invalid event line format => 'r'";
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
                 "insights path.\n";
  }

  cleanup();
}

auto TestValidateStructureInsightsInvalidUtf8(int& failures) -> void {
  const RuntimeTestPaths kPaths = BuildTempTestPaths(
      "time_tracer_android_runtime_validate_structure_invalid_utf8_test");
  RemoveTree(kPaths.test_root);

  const auto cleanup = [&]() -> void { RemoveTree(kPaths.test_root); };

  const std::filesystem::path kRepoRoot = BuildRepoRoot();
  const std::filesystem::path kConfigTomlPath =
      kRepoRoot / "config" / "user" / "behavior.toml";

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

  const std::filesystem::path kSourceRoot =
      kPaths.test_root / "source" / "2026";
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

  const auto kAck = runtime.runtime_api->pipeline().RunValidateStructure(
      {.input_path = kSourceRoot.string()});
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
  TestValidateLogicAllowsSingleAuthoredEventDay(failures);
  TestValidateLogicRejectsBadTimeRangeFixture(failures);
  TestRecordActivityAtomicallyAllowsFirstRecordOnLateMonthDay(failures);
  TestRecordActivityAtomicallyAcceptsWakeKeywordFromConfigOnly(failures);
  TestRecordActivityAtomicallyAcceptsCanonicalActivityToken(failures);
  TestRecordActivityAtomicallyPreservesMultilineRemark(failures);
  TestRecordActivityAtomicallyInsertsBeforeTrailingDaySeparator(failures);
  TestRecordActivityAtomicallyWarnsForOvernightContinuationDay(failures);
  TestRecordActivityAtomicallySkipsCompletenessWarningForCompleteDay(failures);
  TestConvertLogsActualConversionFailure(failures);
  TestValidateStructureInsightsInvalidUtf8(failures);
}

}  // namespace android_runtime_tests
