// infrastructure/tests/report_formatter_parity_tests.cpp
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#include "application/ports/i_report_formatter_registry.hpp"
#include "domain/reports/models/daily_report_data.hpp"
#include "domain/reports/models/period_report_models.hpp"
#include "infrastructure/config/loader/report_config_loader.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/facade/android_static_report_formatter_registrar.hpp"
#include "infrastructure/reports/report_dto_formatter.hpp"

namespace {

// Test fixture code intentionally favors explicit literals and sample labels.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

namespace fs = std::filesystem;

enum class FormatterPipeline { kCliDll, kAndroidStatic };

auto NormalizeNewlines(std::string text) -> std::string {
  std::string normalized;
  normalized.reserve(text.size());
  for (size_t index = 0; index < text.size(); ++index) {
    if (text[index] == '\r') {
      if ((index + 1U < text.size()) && (text[index + 1U] == '\n')) {
        continue;
      }
      normalized.push_back('\n');
      continue;
    }
    normalized.push_back(text[index]);
  }
  return normalized;
}

auto ReadFileText(const fs::path& path) -> std::optional<std::string> {
  std::ifstream input(path);
  if (!input.is_open()) {
    return std::nullopt;
  }
  std::string content((std::istreambuf_iterator<char>(input)),
                      std::istreambuf_iterator<char>());
  return NormalizeNewlines(std::move(content));
}

auto WriteFileText(const fs::path& path, const std::string& content) -> bool {
  std::error_code error;
  fs::create_directories(path.parent_path(), error);
  if (error) {
    return false;
  }

  std::ofstream output(path, std::ios::trunc);
  if (!output.is_open()) {
    return false;
  }
  output << content;
  return static_cast<bool>(output);
}

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
        time_tracer::application::ports::CreateReportFormatterRegistry();
    registry->RegisterFormatters();
  } else {
    auto static_registrar = std::make_shared<
        infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
        infrastructure::reports::AndroidStaticReportFormatterPolicy::
            AllFormats());
    auto registry =
        time_tracer::application::ports::CreateReportFormatterRegistry(
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

auto BuildFirstDiffMessage(const std::string& left, const std::string& right)
    -> std::string {
  const size_t max_length = std::min(left.size(), right.size());
  for (size_t index = 0; index < max_length; ++index) {
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
    return "first mismatch at line " + std::to_string(line) + ", column " +
           std::to_string(column) + ".";
  }

  if (left.size() != right.size()) {
    return "content length differs (expected " + std::to_string(left.size()) +
           ", actual " + std::to_string(right.size()) + ").";
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

  struct CaseOutputs {
    std::string day;
    std::string month;
    std::string week;
    std::string year;
    std::string range;
  };

  auto collect_outputs =
      [&](infrastructure::reports::ReportDtoFormatter& formatter,
          ReportFormat format) -> CaseOutputs {
    CaseOutputs outputs;
    outputs.day = formatter.FormatDaily(daily_report, format);
    outputs.month = formatter.FormatMonthly(monthly_report, format);
    outputs.week = formatter.FormatWeekly(weekly_report, format);
    outputs.year = formatter.FormatYearly(yearly_report, format);
    outputs.range = formatter.FormatPeriod(range_report, format);
    outputs.day = NormalizeNewlines(std::move(outputs.day));
    outputs.month = NormalizeNewlines(std::move(outputs.month));
    outputs.week = NormalizeNewlines(std::move(outputs.week));
    outputs.year = NormalizeNewlines(std::move(outputs.year));
    outputs.range = NormalizeNewlines(std::move(outputs.range));
    return outputs;
  };

  const auto run_case = [&](const std::string& case_name,
                            const fs::path& snapshot_file,
                            const std::string& cli_output,
                            const std::string& android_output) -> void {
    if (update_snapshots) {
      if (!WriteFileText(snapshot_file, cli_output)) {
        ++failures;
        std::cerr << "[FAIL] " << case_name
                  << " failed to update snapshot: " << snapshot_file << '\n';
      }
      if (cli_output != android_output) {
        ++failures;
        std::cerr << "[FAIL] " << case_name
                  << " parity failed while updating snapshot: "
                  << BuildFirstDiffMessage(cli_output, android_output) << '\n';
      }
      return;
    }

    const auto snapshot = ReadFileText(snapshot_file);
    if (!snapshot.has_value()) {
      ++failures;
      std::cerr << "[FAIL] " << case_name
                << " missing snapshot file: " << snapshot_file
                << " (run with TT_UPDATE_FORMATTER_SNAPSHOTS=1)\n";
      return;
    }

    AssertParityAndSnapshot(case_name, *snapshot, cli_output, android_output,
                            failures);
  };

  struct FormatCase {
    ReportFormat format;
    const char* label;
    const char* extension;
  };

  constexpr std::array<FormatCase, 3> kFormats = {{
      {ReportFormat::kMarkdown, "markdown", ".md"},
      {ReportFormat::kLaTeX, "latex", ".tex"},
      {ReportFormat::kTyp, "typst", ".typ"},
  }};

  std::array<CaseOutputs, kFormats.size()> cli_outputs_by_format{};
  std::array<CaseOutputs, kFormats.size()> android_outputs_by_format{};

  try {
    // Register CLI (DLL) creators first. Android static registration then
    // overrides those creators through the same public registry API.
    auto cli_formatter = BuildFormatter(FormatterPipeline::kCliDll, catalog);
    for (size_t index = 0; index < kFormats.size(); ++index) {
      cli_outputs_by_format[index] =
          collect_outputs(*cli_formatter, kFormats[index].format);
    }

    auto android_formatter =
        BuildFormatter(FormatterPipeline::kAndroidStatic, catalog);
    for (size_t index = 0; index < kFormats.size(); ++index) {
      android_outputs_by_format[index] =
          collect_outputs(*android_formatter, kFormats[index].format);
    }
  } catch (const std::exception& exception) {
    std::cerr << "[FAIL] formatter setup threw exception: " << exception.what()
              << '\n';
    return 1;
  }

  for (size_t index = 0; index < kFormats.size(); ++index) {
    const auto& format_case = kFormats[index];
    const auto& cli_outputs = cli_outputs_by_format[index];
    const auto& android_outputs = android_outputs_by_format[index];

    run_case(std::string("daily/") + format_case.label,
             snapshot_root / ("day" + std::string(format_case.extension)),
             cli_outputs.day, android_outputs.day);
    run_case(std::string("monthly/") + format_case.label,
             snapshot_root / ("month" + std::string(format_case.extension)),
             cli_outputs.month, android_outputs.month);
    run_case(std::string("weekly/") + format_case.label,
             snapshot_root / ("week" + std::string(format_case.extension)),
             cli_outputs.week, android_outputs.week);
    run_case(std::string("yearly/") + format_case.label,
             snapshot_root / ("year" + std::string(format_case.extension)),
             cli_outputs.year, android_outputs.year);
    run_case(std::string("range/") + format_case.label,
             snapshot_root / ("range" + std::string(format_case.extension)),
             cli_outputs.range, android_outputs.range);
  }

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

auto main() -> int {
  return RunFormatterParityTests();
}
