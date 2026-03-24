// infrastructure/tests/report_formatter/report_formatter_parity_fixture_tests.cpp
import tracer.core.infrastructure.config.loader;
import tracer.core.domain.reports.models.daily_report_data;
import tracer.core.domain.reports.models.period_report_models;
import tracer.core.domain.reports.models.project_tree;
import tracer.core.domain.reports.types.report_types;
import tracer.core.infrastructure.reporting.dto;

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <utility>

#include "application/ports/reporting/i_report_formatter_registry.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/facade/android_static_report_formatter_registrar.hpp"
#include "infra/tests/report_formatter/report_formatter_parity_internal.hpp"

namespace {

// Test fixture code intentionally favors explicit literals and sample labels.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

namespace fs = std::filesystem;
namespace infra_config = tracer::core::infrastructure::config;
namespace infra_reports = tracer::core::infrastructure::reports;
namespace reporting = tracer::core::domain::modreports;
using tracer::core::domain::modreports::DailyReportData;
using tracer::core::domain::modreports::MonthlyReportData;
using tracer::core::domain::modreports::PeriodReportData;
using tracer::core::domain::modreports::ReportFormat;
using tracer::core::domain::modreports::TimeRecord;
using tracer::core::domain::modreports::WeeklyReportData;
using tracer::core::domain::modreports::YearlyReportData;
using report_formatter_parity_internal::CaseOutputs;
using report_formatter_parity_internal::ParityOutputs;

enum class FormatterPipeline { kDefaultRegistry, kAndroidStatic };

auto BuildRepoRoot() -> fs::path {
  return fs::path(__FILE__)
      .parent_path()   // report_formatter
      .parent_path()   // tests
      .parent_path()   // infrastructure
      .parent_path()   // tests
      .parent_path()   // tracer_core
      .parent_path()   // libs
      .parent_path();  // repo root
}

auto BuildReportCatalog(const fs::path& repo_root) -> ReportCatalog {
  ReportCatalog catalog;

  const fs::path report_config_root =
      repo_root / "assets" / "tracer_core" / "config" / "reports";
  const fs::path markdown_config_dir = report_config_root / "markdown";
  const fs::path latex_config_dir = report_config_root / "latex";
  const fs::path typst_config_dir = report_config_root / "typst";

  catalog.loaded_reports.markdown.day = infra_config::ReportConfigLoader::
      LoadDailyMdConfig(markdown_config_dir / "day.toml");
  catalog.loaded_reports.markdown.month =
      infra_config::ReportConfigLoader::LoadMonthlyMdConfig(
          markdown_config_dir / "month.toml");
  catalog.loaded_reports.markdown.period =
      infra_config::ReportConfigLoader::LoadPeriodMdConfig(
          markdown_config_dir / "period.toml");
  catalog.loaded_reports.markdown.week =
      infra_config::ReportConfigLoader::LoadWeeklyMdConfig(
          markdown_config_dir / "week.toml");
  catalog.loaded_reports.markdown.year =
      infra_config::ReportConfigLoader::LoadYearlyMdConfig(
          markdown_config_dir / "year.toml");

  catalog.loaded_reports.latex.day =
      infra_config::ReportConfigLoader::LoadDailyTexConfig(
          latex_config_dir / "day.toml");
  catalog.loaded_reports.latex.month =
      infra_config::ReportConfigLoader::LoadMonthlyTexConfig(
          latex_config_dir / "month.toml");
  catalog.loaded_reports.latex.period =
      infra_config::ReportConfigLoader::LoadPeriodTexConfig(
          latex_config_dir / "period.toml");
  catalog.loaded_reports.latex.week =
      infra_config::ReportConfigLoader::LoadWeeklyTexConfig(
          latex_config_dir / "week.toml");
  catalog.loaded_reports.latex.year =
      infra_config::ReportConfigLoader::LoadYearlyTexConfig(
          latex_config_dir / "year.toml");

  catalog.loaded_reports.typst.day =
      infra_config::ReportConfigLoader::LoadDailyTypConfig(
          typst_config_dir / "day.toml");
  catalog.loaded_reports.typst.month =
      infra_config::ReportConfigLoader::LoadMonthlyTypConfig(
          typst_config_dir / "month.toml");
  catalog.loaded_reports.typst.period =
      infra_config::ReportConfigLoader::LoadPeriodTypConfig(
          typst_config_dir / "period.toml");
  catalog.loaded_reports.typst.week =
      infra_config::ReportConfigLoader::LoadWeeklyTypConfig(
          typst_config_dir / "week.toml");
  catalog.loaded_reports.typst.year =
      infra_config::ReportConfigLoader::LoadYearlyTypConfig(
          typst_config_dir / "year.toml");

  return catalog;
}

auto BuildFormatter(FormatterPipeline pipeline, const ReportCatalog& catalog)
    -> std::unique_ptr<infra_reports::ReportDtoFormatter> {
  if (pipeline == FormatterPipeline::kDefaultRegistry) {
    auto registry =
        tracer_core::application::ports::CreateReportFormatterRegistry();
    registry->RegisterFormatters();
  } else {
    auto static_registrar = std::make_shared<
        infrastructure::reports::AndroidStaticReportFormatterRegistrar>(
        infrastructure::reports::AndroidStaticReportFormatterPolicy::AllFormats());
    auto registry =
        tracer_core::application::ports::CreateReportFormatterRegistry(
            static_registrar);
    registry->RegisterFormatters();
  }

  return std::make_unique<infra_reports::ReportDtoFormatter>(catalog);
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
  report.metadata.wake_anchor = "0";
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
  report.wake_anchor_true_days = sleep_days;
  report.exercise_true_days = exercise_days;
  report.cardio_true_days = cardio_days;
  report.anaerobic_true_days = anaerobic_days;
  report.is_valid = true;
  report.project_tree = BuildRangeProjectTree();
  return report;
}

auto CollectOutputs(infra_reports::ReportDtoFormatter& formatter,
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

// NOLINTEND(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

}  // namespace

namespace report_formatter_parity_internal {

auto RunFormatterParityTests() -> int {
  int failures = 0;

  const fs::path repo_root = BuildRepoRoot();
  const fs::path snapshot_root =
      repo_root / "test" / "golden" / "report_formatter_parity" / "v1";
  const bool update_snapshots =
      std::getenv("TT_UPDATE_FORMATTER_SNAPSHOTS") != nullptr;

  ReportCatalog catalog;
  try {
    catalog = BuildReportCatalog(repo_root);
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
    auto cli_formatter =
        BuildFormatter(FormatterPipeline::kDefaultRegistry, catalog);
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

  RunMarkdownSnapshotCases(snapshot_root, outputs, update_snapshots, failures);
  RunLatexSnapshotCases(snapshot_root, outputs, update_snapshots, failures);
  RunTypstSnapshotCases(snapshot_root, outputs, update_snapshots, failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_formatter_parity_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_formatter_parity_tests failures: "
            << failures << '\n';
  return 1;
}

}  // namespace report_formatter_parity_internal
