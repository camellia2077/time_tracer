// infrastructure/tests/report_formatter/report_formatter_parity_md_tests.cpp
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#include <sodium.h>

#include "application/ports/i_report_formatter_registry.hpp"
#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"
#include "infrastructure/tests/report_formatter/report_formatter_parity_internal.hpp"

#ifndef TT_ENABLE_HEAVY_DIAGNOSTICS
#define TT_ENABLE_HEAVY_DIAGNOSTICS 0
#endif

namespace {

// Test fixture code intentionally favors explicit literals and sample labels.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

namespace fs = std::filesystem;
using report_formatter_parity_internal::CaseOutputs;
using report_formatter_parity_internal::ParityOutputs;

enum class FormatterPipeline { kCliDll, kAndroidStatic };

auto BuildRepoRoot() -> fs::path {
  return fs::path(__FILE__)
      .parent_path()   // tests
      .parent_path()   // infrastructure
      .parent_path()   // src
      .parent_path()   // time_tracer
      .parent_path();  // apps
}

auto BuildExecutableDirectory() -> fs::path {
#ifdef _WIN32
  std::wstring buffer(static_cast<size_t>(MAX_PATH), L'\0');
  DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                    static_cast<DWORD>(buffer.size()));
  while ((length >= buffer.size()) && (length != 0U)) {
    buffer.resize(buffer.size() * 2U);
    length = GetModuleFileNameW(nullptr, buffer.data(),
                                static_cast<DWORD>(buffer.size()));
  }
  if (length != 0U) {
    buffer.resize(length);
    return fs::path(buffer).parent_path();
  }
#endif
  return fs::current_path();
}

auto BuildReportCatalog(const fs::path& repo_root, const fs::path& plugin_dir)
    -> ReportCatalog {
  ReportCatalog catalog;
  catalog.plugin_dir_path = plugin_dir;

  const fs::path markdown_config_dir =
      repo_root / "time_tracer" / "config" / "reports" / "markdown";
  const fs::path latex_config_dir =
      repo_root / "time_tracer" / "config" / "reports" / "latex";
  const fs::path typst_config_dir =
      repo_root / "time_tracer" / "config" / "reports" / "typst";

  catalog.loaded_reports.markdown.day =
      ReportConfigLoader::LoadDailyMdConfig(markdown_config_dir / "day.toml");
  catalog.loaded_reports.markdown.month =
      ReportConfigLoader::LoadMonthlyMdConfig(markdown_config_dir /
                                              "month.toml");
  catalog.loaded_reports.markdown.period =
      ReportConfigLoader::LoadPeriodMdConfig(markdown_config_dir /
                                             "period.toml");
  catalog.loaded_reports.markdown.week =
      ReportConfigLoader::LoadWeeklyMdConfig(markdown_config_dir / "week.toml");
  catalog.loaded_reports.markdown.year =
      ReportConfigLoader::LoadYearlyMdConfig(markdown_config_dir / "year.toml");

  catalog.loaded_reports.latex.day =
      ReportConfigLoader::LoadDailyTexConfig(latex_config_dir / "day.toml");
  catalog.loaded_reports.latex.month =
      ReportConfigLoader::LoadMonthlyTexConfig(latex_config_dir / "month.toml");
  catalog.loaded_reports.latex.period =
      ReportConfigLoader::LoadPeriodTexConfig(latex_config_dir / "period.toml");
  catalog.loaded_reports.latex.week =
      ReportConfigLoader::LoadWeeklyTexConfig(latex_config_dir / "week.toml");
  catalog.loaded_reports.latex.year =
      ReportConfigLoader::LoadYearlyTexConfig(latex_config_dir / "year.toml");

  catalog.loaded_reports.typst.day =
      ReportConfigLoader::LoadDailyTypConfig(typst_config_dir / "day.toml");
  catalog.loaded_reports.typst.month =
      ReportConfigLoader::LoadMonthlyTypConfig(typst_config_dir / "month.toml");
  catalog.loaded_reports.typst.period =
      ReportConfigLoader::LoadPeriodTypConfig(typst_config_dir / "period.toml");
  catalog.loaded_reports.typst.week =
      ReportConfigLoader::LoadWeeklyTypConfig(typst_config_dir / "week.toml");
  catalog.loaded_reports.typst.year =
      ReportConfigLoader::LoadYearlyTypConfig(typst_config_dir / "year.toml");

  return catalog;
}

auto BuildFormatter(FormatterPipeline pipeline, const ReportCatalog& catalog)
    -> std::unique_ptr<infrastructure::reports::ReportDtoFormatter> {
  if (pipeline == FormatterPipeline::kCliDll) {
    auto registry =
        tracer_core::application::ports::CreateReportFormatterRegistry();
    registry->RegisterFormatters();
  } else {
    auto static_registrar = std::make_shared<
        infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
        infrastructure::reports::AndroidStaticReportFormatterPolicy::
            AllFormats());
    auto registry =
        tracer_core::application::ports::CreateReportFormatterRegistry(
            static_registrar);
    registry->RegisterFormatters();
  }

  return std::make_unique<infrastructure::reports::ReportDtoFormatter>(catalog);
}

auto BuildDailyProjectTree() -> reporting::ProjectTree {
  reporting::ProjectNode work{};
  work.duration = 7800;
  work.children["Coding"] = reporting::ProjectNode{.duration = 4200};
  work.children["Review"] = reporting::ProjectNode{.duration = 3600};

  reporting::ProjectNode life{};
  life.duration = 4800;
  life.children["Reading"] = reporting::ProjectNode{.duration = 3000};
  life.children["Exercise"] = reporting::ProjectNode{.duration = 1800};

  reporting::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildRangeProjectTree() -> reporting::ProjectTree {
  reporting::ProjectNode work{};
  work.duration = 28800;
  work.children["Coding"] = reporting::ProjectNode{.duration = 18000};
  work.children["Review"] = reporting::ProjectNode{.duration = 10800};

  reporting::ProjectNode life{};
  life.duration = 25200;
  life.children["Reading"] = reporting::ProjectNode{.duration = 18000};
  life.children["Exercise"] = reporting::ProjectNode{.duration = 7200};

  reporting::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildDailyFixture() -> DailyReportData {
  DailyReportData report;
  report.date = "2021-01-03";
  report.metadata.status = "1";
  report.metadata.sleep = "0";
  report.metadata.exercise = "1";
  report.metadata.getup_time = "07:30";
  report.metadata.remark = "Deep work\nEvening workout";
  report.total_duration = 12600;
  report.stats["sleep_total_time"] = 25200;
  report.stats["study_time"] = 3600;
  report.stats["total_exercise_time"] = 1800;
  report.stats["anaerobic_time"] = 900;
  report.stats["cardio_time"] = 900;
  report.stats["grooming_time"] = 1200;
  report.stats["recreation_time"] = 2400;
  report.stats["recreation_zhihu_time"] = 600;
  report.stats["recreation_bilibili_time"] = 1200;
  report.stats["recreation_douyin_time"] = 600;
  report.detailed_records.push_back(TimeRecord{
      .start_time = "08:00",
      .end_time = "09:10",
      .project_path = "Work_Coding",
      .duration_seconds = 4200,
      .activityRemark = std::optional<std::string>{"Feature refactor"},
  });
  report.detailed_records.push_back(TimeRecord{
      .start_time = "09:20",
      .end_time = "10:20",
      .project_path = "Work_Review",
      .duration_seconds = 3600,
      .activityRemark = std::nullopt,
  });
  report.detailed_records.push_back(TimeRecord{
      .start_time = "20:00",
      .end_time = "20:30",
      .project_path = "Life_Exercise",
      .duration_seconds = 1800,
      .activityRemark = std::optional<std::string>{"Cardio"},
  });
  report.project_tree = BuildDailyProjectTree();
  return report;
}

template <typename RangeReportType>
auto BuildRangeFixture(const std::string& range_label,
                       const std::string& start_date,
                       const std::string& end_date, int requested_days,
                       int actual_days, int status_days, int sleep_days,
                       int exercise_days, int cardio_days, int anaerobic_days)
    -> RangeReportType {
  RangeReportType report;
  report.range_label = range_label;
  report.start_date = start_date;
  report.end_date = end_date;
  report.requested_days = requested_days;
  report.total_duration = 54000;
  report.actual_days = actual_days;
  report.status_true_days = status_days;
  report.sleep_true_days = sleep_days;
  report.exercise_true_days = exercise_days;
  report.cardio_true_days = cardio_days;
  report.anaerobic_true_days = anaerobic_days;
  report.is_valid = true;
  report.project_tree = BuildRangeProjectTree();
  return report;
}

auto CollectOutputs(infrastructure::reports::ReportDtoFormatter& formatter,
                    const DailyReportData& daily_report,
                    const MonthlyReportData& monthly_report,
                    const WeeklyReportData& weekly_report,
                    const YearlyReportData& yearly_report,
                    const PeriodReportData& range_report, ReportFormat format)
    -> CaseOutputs {
  CaseOutputs outputs;
  outputs.day = formatter.FormatDaily(daily_report, format);
  outputs.month = formatter.FormatMonthly(monthly_report, format);
  outputs.week = formatter.FormatWeekly(weekly_report, format);
  outputs.year = formatter.FormatYearly(yearly_report, format);
  outputs.range = formatter.FormatPeriod(range_report, format);
  return outputs;
}

auto RunFormatterParityTests() -> int {
  int failures = 0;

  const fs::path repo_root = BuildRepoRoot();
  const fs::path snapshot_root = repo_root / "time_tracer" / "src" /
                                 "infrastructure" / "tests" / "golden" /
                                 "formatter_parity";
  const fs::path executable_dir = BuildExecutableDirectory();
  const fs::path plugin_dir = executable_dir / "plugins";
  const bool update_snapshots =
      std::getenv("TT_UPDATE_FORMATTER_SNAPSHOTS") != nullptr;

  if (!fs::exists(plugin_dir)) {
    std::cerr << "[FAIL] Plugin directory not found: " << plugin_dir << '\n';
    return 1;
  }

  ReportCatalog catalog;
  try {
    catalog = BuildReportCatalog(repo_root, plugin_dir);
  } catch (const std::exception& exception) {
    std::cerr << "[FAIL] Failed to load report config for parity tests: "
              << exception.what() << '\n';
    return 1;
  }

  const DailyReportData daily_report = BuildDailyFixture();
  const MonthlyReportData monthly_report = BuildRangeFixture<MonthlyReportData>(
      "2026-01", "2026-01-01", "2026-01-31", 31, 6, 5, 4, 3, 2, 2);
  const WeeklyReportData weekly_report = BuildRangeFixture<WeeklyReportData>(
      "2026-W05", "2026-01-27", "2026-02-02", 7, 4, 3, 2, 2, 1, 1);
  const YearlyReportData yearly_report = BuildRangeFixture<YearlyReportData>(
      "2026", "2026-01-01", "2026-12-31", 365, 120, 90, 80, 60, 40, 30);
  const PeriodReportData range_report = BuildRangeFixture<PeriodReportData>(
      "Last 10 days", "2026-01-01", "2026-01-10", 10, 5, 4, 3, 2, 1, 2);

  ParityOutputs outputs;
  try {
    auto cli_formatter = BuildFormatter(FormatterPipeline::kCliDll, catalog);
    outputs.cli_by_format[0] = CollectOutputs(
        *cli_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kMarkdown);
    outputs.cli_by_format[1] = CollectOutputs(
        *cli_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kLaTeX);
    outputs.cli_by_format[2] = CollectOutputs(
        *cli_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kTyp);

    auto android_formatter =
        BuildFormatter(FormatterPipeline::kAndroidStatic, catalog);
    outputs.android_by_format[0] = CollectOutputs(
        *android_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kMarkdown);
    outputs.android_by_format[1] = CollectOutputs(
        *android_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kLaTeX);
    outputs.android_by_format[2] = CollectOutputs(
        *android_formatter, daily_report, monthly_report, weekly_report,
        yearly_report, range_report, ReportFormat::kTyp);
  } catch (const std::exception& exception) {
    std::cerr << "[FAIL] formatter setup threw exception: " << exception.what()
              << '\n';
    return 1;
  }

  report_formatter_parity_internal::RunMarkdownSnapshotCases(
      snapshot_root, outputs, update_snapshots, failures);
  report_formatter_parity_internal::RunLatexSnapshotCases(
      snapshot_root, outputs, update_snapshots, failures);
  report_formatter_parity_internal::RunTypstSnapshotCases(
      snapshot_root, outputs, update_snapshots, failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_formatter_parity_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_formatter_parity_tests failures: "
            << failures << '\n';
  return 1;
}

// NOLINTEND(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)
}  // namespace

namespace report_formatter_parity_internal {

namespace {

auto ToHexLower(const unsigned char* bytes, size_t size) -> std::string {
  constexpr char kHex[] = "0123456789abcdef";
  std::string text(size * 2U, '\0');
  for (size_t index = 0; index < size; ++index) {
    const unsigned char value = bytes[index];
    text[index * 2U] = kHex[(value >> 4U) & 0x0FU];
    text[index * 2U + 1U] = kHex[value & 0x0FU];
  }
  return text;
}

auto ComputeSha256Hex(std::string_view text) -> std::string {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return "sha256_error";
  }
  return ToHexLower(digest.data(), digest.size());
}

#if TT_ENABLE_HEAVY_DIAGNOSTICS
auto BuildDiffContext(std::string_view text, size_t offset) -> std::string {
  constexpr size_t kRadius = 24;
  const size_t begin = (offset > kRadius) ? (offset - kRadius) : 0U;
  const size_t end = std::min(text.size(), offset + kRadius);
  std::string snippet(text.substr(begin, end - begin));
  for (char& value : snippet) {
    if (value == '\n' || value == '\r' || value == '\t') {
      value = ' ';
    }
  }
  return snippet;
}
#endif

}  // namespace

auto ReadFileText(const std::filesystem::path& path)
    -> std::optional<std::string> {
  std::ifstream input(path, std::ios::binary);
  if (!input.is_open()) {
    return std::nullopt;
  }
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  return content;
}

auto WriteFileText(const std::filesystem::path& path,
                   const std::string& content) -> bool {
  std::error_code error;
  std::filesystem::create_directories(path.parent_path(), error);
  if (error) {
    return false;
  }

  std::ofstream output(path, std::ios::trunc | std::ios::binary);
  if (!output.is_open()) {
    return false;
  }
  output.write(content.data(), static_cast<std::streamsize>(content.size()));
  return static_cast<bool>(output);
}

auto BuildFirstDiffMessage(const std::string& left, const std::string& right)
    -> std::string {
  const size_t kMaxLength = std::min(left.size(), right.size());
  for (size_t index = 0; index < kMaxLength; ++index) {
    if (left[index] == right[index]) {
      continue;
    }

    int line = 1;
    int column = 1;
    for (size_t pos = 0; pos < index; ++pos) {
      if (left[pos] == '\n') {
        ++line;
        column = 1;
      } else {
        ++column;
      }
    }
    std::ostringstream output;
    output << "first mismatch at line " << line << ", column " << column
           << ", byte offset " << index << "; "
           << "left_sha256=" << ComputeSha256Hex(left) << ", "
           << "right_sha256=" << ComputeSha256Hex(right) << ".";
#if TT_ENABLE_HEAVY_DIAGNOSTICS
    output << " left_context=\"" << BuildDiffContext(left, index)
           << "\", right_context=\"" << BuildDiffContext(right, index) << "\".";
#endif
    return output.str();
  }

  if (left.size() != right.size()) {
    std::ostringstream output;
    output << "content length differs (expected " << left.size() << ", actual "
           << right.size() << "); "
           << "left_sha256=" << ComputeSha256Hex(left) << ", "
           << "right_sha256=" << ComputeSha256Hex(right) << ".";
    return output.str();
  }
  return "unknown mismatch.";
}

auto AssertParityAndSnapshot(const std::string& case_name,
                             const std::string& snapshot_content,
                             const std::string& cli_content,
                             const std::string& android_content, int& failures)
    -> void {
  if (cli_content != android_content) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " parity failed between CLI and Android: "
              << BuildFirstDiffMessage(cli_content, android_content) << '\n';
  }

  if (snapshot_content != cli_content) {
    ++failures;
    std::cerr << "[FAIL] " << case_name << " snapshot mismatch: "
              << BuildFirstDiffMessage(snapshot_content, cli_content) << '\n';
  }
}

auto RunCaseWithSnapshot(const std::string& case_name,
                         const std::filesystem::path& snapshot_file,
                         const std::string& cli_output,
                         const std::string& android_output,
                         bool update_snapshots, int& failures) -> void {
  // Step 3.5 hard gate: raw bytes and their SHA-256 must be equal
  // cross-pipeline.
  if (cli_output != android_output) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " raw-byte parity failed between CLI and Android: "
              << BuildFirstDiffMessage(cli_output, android_output) << '\n';
  }

  const std::string cli_hash = ComputeSha256Hex(cli_output);
  const std::string android_hash = ComputeSha256Hex(android_output);
  if (cli_hash != android_hash) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " hash parity failed between CLI and Android: cli_sha256="
              << cli_hash << ", android_sha256=" << android_hash << '\n';
  }

  const std::string cli_snapshot_text = cli_output;
  const std::string android_snapshot_text = android_output;

  if (update_snapshots) {
    if (!WriteFileText(snapshot_file, cli_snapshot_text)) {
      ++failures;
      std::cerr << "[FAIL] " << case_name
                << " failed to update snapshot: " << snapshot_file << '\n';
    }
    return;
  }

  const auto kSnapshot = ReadFileText(snapshot_file);
  if (!kSnapshot.has_value()) {
    ++failures;
    std::cerr << "[FAIL] " << case_name
              << " missing snapshot file: " << snapshot_file
              << " (run with TT_UPDATE_FORMATTER_SNAPSHOTS=1)\n";
    return;
  }

  AssertParityAndSnapshot(case_name, *kSnapshot, cli_snapshot_text,
                          android_snapshot_text, failures);
}

auto RunFormatSnapshotCases(const std::string& format_label,
                            const std::string& extension,
                            const std::filesystem::path& snapshot_root,
                            const CaseOutputs& cli_outputs,
                            const CaseOutputs& android_outputs,
                            bool update_snapshots, int& failures) -> void {
  RunCaseWithSnapshot("daily/" + format_label,
                      snapshot_root / ("day" + extension), cli_outputs.day,
                      android_outputs.day, update_snapshots, failures);
  RunCaseWithSnapshot("monthly/" + format_label,
                      snapshot_root / ("month" + extension), cli_outputs.month,
                      android_outputs.month, update_snapshots, failures);
  RunCaseWithSnapshot("weekly/" + format_label,
                      snapshot_root / ("week" + extension), cli_outputs.week,
                      android_outputs.week, update_snapshots, failures);
  RunCaseWithSnapshot("yearly/" + format_label,
                      snapshot_root / ("year" + extension), cli_outputs.year,
                      android_outputs.year, update_snapshots, failures);
  RunCaseWithSnapshot("range/" + format_label,
                      snapshot_root / ("range" + extension), cli_outputs.range,
                      android_outputs.range, update_snapshots, failures);
}

auto RunMarkdownSnapshotCases(const std::filesystem::path& snapshot_root,
                              const ParityOutputs& outputs,
                              bool update_snapshots, int& failures) -> void {
  constexpr size_t kMarkdownIndex = 0;
  RunFormatSnapshotCases(
      "markdown", ".md", snapshot_root, outputs.cli_by_format[kMarkdownIndex],
      outputs.android_by_format[kMarkdownIndex], update_snapshots, failures);
}

}  // namespace report_formatter_parity_internal

auto main() -> int {
  return RunFormatterParityTests();
}
