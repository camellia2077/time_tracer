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
#include <string_view>
#include <utility>

#include "application/ports/reporting/i_report_formatter_registry.hpp"
#include "domain/model/time_data_models.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/reporting/facade/android_static_report_formatter_registrar.hpp"
#include "infra/reporting/shared/utils/format/report_string_utils.hpp"
#include "infra/reporting/shared/utils/format/time_format.hpp"
#include "infra/tests/report_formatter/report_formatter_parity_internal.hpp"

namespace {

// Test fixture code intentionally favors explicit literals and sample labels.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

namespace fs = std::filesystem;
namespace infra_config = tracer::core::infrastructure::config;
namespace infra_reports = tracer::core::infrastructure::reports;
namespace reporting = tracer::core::domain::modreports;
using report_formatter_parity_internal::CaseOutputs;
using report_formatter_parity_internal::ParityOutputs;
using tracer::core::domain::modreports::DailyReportData;
using tracer::core::domain::modreports::MonthlyReportData;
using tracer::core::domain::modreports::PeriodReportData;
using tracer::core::domain::modreports::ReportFormat;
using tracer::core::domain::modreports::TimeRecord;
using tracer::core::domain::modreports::WeeklyReportData;
using tracer::core::domain::modreports::YearlyReportData;

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
      repo_root / "config" / "program" / "reports";
  const fs::path markdown_config_dir = report_config_root / "markdown";
  const fs::path markdown_default_config_dir = markdown_config_dir / "en";
  const fs::path latex_config_dir = report_config_root / "latex";
  const fs::path typst_config_dir = report_config_root / "typst";

  catalog.loaded_reports.markdown.day =
      infra_config::ReportConfigLoader::LoadDailyMdConfig(
          markdown_default_config_dir / "day.toml");
  catalog.loaded_reports.markdown.month =
      infra_config::ReportConfigLoader::LoadMonthlyMdConfig(
          markdown_default_config_dir / "month.toml");
  catalog.loaded_reports.markdown.period =
      infra_config::ReportConfigLoader::LoadPeriodMdConfig(
          markdown_default_config_dir / "period.toml");
  catalog.loaded_reports.markdown.week =
      infra_config::ReportConfigLoader::LoadWeeklyMdConfig(
          markdown_default_config_dir / "week.toml");
  catalog.loaded_reports.markdown.year =
      infra_config::ReportConfigLoader::LoadYearlyMdConfig(
          markdown_default_config_dir / "year.toml");

  for (const std::string_view locale : {"en", "zh", "ja"}) {
    const fs::path locale_dir = markdown_config_dir / std::string(locale);
    MarkdownReportConfigs localized;
    localized.day = infra_config::ReportConfigLoader::LoadDailyMdConfig(
        locale_dir / "day.toml");
    localized.month = infra_config::ReportConfigLoader::LoadMonthlyMdConfig(
        locale_dir / "month.toml");
    localized.period = infra_config::ReportConfigLoader::LoadPeriodMdConfig(
        locale_dir / "period.toml");
    localized.week = infra_config::ReportConfigLoader::LoadWeeklyMdConfig(
        locale_dir / "week.toml");
    localized.year = infra_config::ReportConfigLoader::LoadYearlyMdConfig(
        locale_dir / "year.toml");
    catalog.loaded_reports.markdown_locales.emplace(std::string(locale),
                                                    std::move(localized));
  }

  catalog.loaded_reports.latex.day =
      infra_config::ReportConfigLoader::LoadDailyTexConfig(latex_config_dir /
                                                           "day.toml");
  catalog.loaded_reports.latex.month =
      infra_config::ReportConfigLoader::LoadMonthlyTexConfig(latex_config_dir /
                                                             "month.toml");
  catalog.loaded_reports.latex.period =
      infra_config::ReportConfigLoader::LoadPeriodTexConfig(latex_config_dir /
                                                            "period.toml");
  catalog.loaded_reports.latex.week =
      infra_config::ReportConfigLoader::LoadWeeklyTexConfig(latex_config_dir /
                                                            "week.toml");
  catalog.loaded_reports.latex.year =
      infra_config::ReportConfigLoader::LoadYearlyTexConfig(latex_config_dir /
                                                            "year.toml");

  catalog.loaded_reports.typst.day =
      infra_config::ReportConfigLoader::LoadDailyTypConfig(typst_config_dir /
                                                           "day.toml");
  catalog.loaded_reports.typst.month =
      infra_config::ReportConfigLoader::LoadMonthlyTypConfig(typst_config_dir /
                                                             "month.toml");
  catalog.loaded_reports.typst.period =
      infra_config::ReportConfigLoader::LoadPeriodTypConfig(typst_config_dir /
                                                            "period.toml");
  catalog.loaded_reports.typst.week =
      infra_config::ReportConfigLoader::LoadWeeklyTypConfig(typst_config_dir /
                                                            "week.toml");
  catalog.loaded_reports.typst.year =
      infra_config::ReportConfigLoader::LoadYearlyTypConfig(typst_config_dir /
                                                            "year.toml");

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
        infrastructure::reports::AndroidStaticReportFormatterPolicy::
            AllFormats());
    auto registry =
        tracer_core::application::ports::CreateReportFormatterRegistry(
            static_registrar);
    registry->RegisterFormatters();
  }

  return std::make_unique<infra_reports::ReportDtoFormatter>(catalog);
}

auto BuildDailyProjectTree() -> reporting::ProjectTree {
  reporting::ProjectNode work{.duration = 7800, .occurrence_count = 2};
  work.children["Coding"] =
      reporting::ProjectNode{.duration = 4200, .occurrence_count = 1};
  work.children["Review"] =
      reporting::ProjectNode{.duration = 3600, .occurrence_count = 1};

  reporting::ProjectNode life{.duration = 4800, .occurrence_count = 2};
  life.children["Reading"] =
      reporting::ProjectNode{.duration = 3000, .occurrence_count = 1};
  life.children["Exercise"] =
      reporting::ProjectNode{.duration = 1800, .occurrence_count = 1};

  reporting::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildRangeProjectTree() -> reporting::ProjectTree {
  reporting::ProjectNode work{.duration = 28800, .occurrence_count = 5};
  work.children["Coding"] =
      reporting::ProjectNode{.duration = 18000, .occurrence_count = 3};
  work.children["Review"] =
      reporting::ProjectNode{.duration = 10800, .occurrence_count = 2};

  reporting::ProjectNode life{.duration = 25200, .occurrence_count = 4};
  life.children["Reading"] =
      reporting::ProjectNode{.duration = 18000, .occurrence_count = 3};
  life.children["Exercise"] =
      reporting::ProjectNode{.duration = 7200, .occurrence_count = 1};

  reporting::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildDailyFixture() -> DailyReportData {
  DailyReportData report;
  report.date = "2021-01-03";
  report.metadata.statuses = {
      {.id = "study", .label = "Study", .value = true},
      {.id = "exercise", .label = "Exercise", .value = true},
  };
  report.metadata.getup_time = "07:30";
  report.metadata.remark = "Deep work\nEvening workout";
  report.total_duration = 12600;
  report.activity_count = 3;
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
      .activityRemark = std::optional<std::string>{"Feature refactor\nFollow-up"},
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
                       int actual_days, int status_days,
                       int exercise_days,
                       int cardio_days, int anaerobic_days)
    -> RangeReportType {
  RangeReportType report;
  report.range_label = range_label;
  report.start_date = start_date;
  report.end_date = end_date;
  report.requested_days = requested_days;
  report.total_duration = 54000;
  report.actual_days = actual_days;
  report.matched_record_count = actual_days * 2;
  report.status_true_days = status_days;
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

auto ExpectContains(std::string_view case_name, std::string_view content,
                    std::string_view expected, int& failures) -> void {
  if (content.find(expected) != std::string_view::npos) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << case_name << " should contain `" << expected
            << "`.\n";
}

auto ExpectNotContains(std::string_view case_name, std::string_view content,
                       std::string_view unexpected, int& failures) -> void {
  if (content.find(unexpected) == std::string_view::npos) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << case_name << " should not contain `"
            << unexpected << "`.\n";
}

auto CheckBooleanMetadataLabels(const ParityOutputs& outputs,
                                int& failures) -> void {
  ExpectContains("positive boolean summary label",
                 FormatBooleanCountLabel("Cardio Days (True)", 16),
                 "Cardio Days", failures);
  ExpectNotContains("positive boolean summary label",
                    FormatBooleanCountLabel("Cardio Days (True)", 16),
                    "Cardio Days (True)", failures);
  ExpectContains("zero boolean summary label",
                 FormatBooleanCountLabel("Cardio Days (True)", 0),
                 "Cardio Days (False)", failures);

  const auto& markdown = outputs.cli_by_format[0];
  ExpectNotContains("monthly markdown boolean metadata", markdown.month,
                    "recorded_coverage_ratio", failures);
  ExpectNotContains("weekly markdown boolean metadata", markdown.week,
                    "recorded_coverage_ratio", failures);
  ExpectNotContains("yearly markdown boolean metadata", markdown.year,
                    "recorded_coverage_ratio", failures);
  ExpectNotContains("range markdown boolean metadata", markdown.range,
                    "recorded_coverage_ratio", failures);
}

auto CheckDailyActivityCountLabels(const ParityOutputs& outputs,
                                   int& failures) -> void {
  constexpr std::string_view kMarkdownExpected = "- **Activity Count**: 3";
  constexpr std::string_view kLatexExpected =
      "\\textbf{Activity Count}: 3";
  constexpr std::string_view kTypstExpected = "+ *Activity Count:* 3";
  ExpectContains("daily markdown activity count", outputs.cli_by_format[0].day,
                 kMarkdownExpected, failures);
  ExpectContains("daily Android markdown activity count",
                 outputs.android_by_format[0].day, kMarkdownExpected, failures);
  ExpectContains("daily latex activity count", outputs.cli_by_format[1].day,
                 kLatexExpected, failures);
  ExpectContains("daily Android latex activity count",
                 outputs.android_by_format[1].day, kLatexExpected, failures);
  ExpectContains("daily typst activity count", outputs.cli_by_format[2].day,
                 kTypstExpected, failures);
  ExpectContains("daily Android typst activity count",
                 outputs.android_by_format[2].day, kTypstExpected, failures);
}

auto CheckDailyStatusLabelsArePresent(const ParityOutputs& outputs,
                                      int& failures) -> void {
  constexpr std::string_view kMarkdownStudy = "- **Study**: true";
  constexpr std::string_view kMarkdownExercise = "- **Exercise**: true";
  constexpr std::string_view kLatexStudy = "\\textbf{Study}: true";
  constexpr std::string_view kLatexExercise = "\\textbf{Exercise}: true";
  constexpr std::string_view kTypstStudy = "+ *Study:* true";
  constexpr std::string_view kTypstExercise = "+ *Exercise:* true";

  ExpectContains("CLI daily Markdown Study status", outputs.cli_by_format[0].day,
                 kMarkdownStudy, failures);
  ExpectContains("CLI daily Markdown Exercise status",
                 outputs.cli_by_format[0].day, kMarkdownExercise, failures);
  ExpectContains("CLI daily LaTeX Study status", outputs.cli_by_format[1].day,
                 kLatexStudy, failures);
  ExpectContains("CLI daily LaTeX Exercise status",
                 outputs.cli_by_format[1].day, kLatexExercise, failures);
  ExpectContains("CLI daily Typst Study status", outputs.cli_by_format[2].day,
                 kTypstStudy, failures);
  ExpectContains("CLI daily Typst Exercise status",
                 outputs.cli_by_format[2].day, kTypstExercise, failures);

  ExpectContains("Android daily Markdown Study status",
                 outputs.android_by_format[0].day, kMarkdownStudy, failures);
  ExpectContains("Android daily Markdown Exercise status",
                 outputs.android_by_format[0].day, kMarkdownExercise,
                 failures);
  ExpectContains("Android daily LaTeX Study status",
                 outputs.android_by_format[1].day, kLatexStudy, failures);
  ExpectContains("Android daily LaTeX Exercise status",
                 outputs.android_by_format[1].day, kLatexExercise, failures);
  ExpectContains("Android daily Typst Study status",
                 outputs.android_by_format[2].day, kTypstStudy, failures);
  ExpectContains("Android daily Typst Exercise status",
                 outputs.android_by_format[2].day, kTypstExercise, failures);
}

auto CheckDailyTimelineFormatting(infra_reports::ReportDtoFormatter& formatter,
                                  const DailyReportData& source_report,
                                  int& failures) -> void {
  DailyReportData timeline_report = source_report;
  timeline_report.activity_count = 2;
  timeline_report.detailed_records = {
      TimeRecord{.start_time = "08:00:12",
                 .end_time = "09:10:05",
                 .project_path = "Work_Coding",
                 .duration_seconds = 4200},
      TimeRecord{.start_time = "09:20:00",
                 .end_time = "10:20:00",
                 .project_path = "Work_Review",
                 .duration_seconds = 3600},
  };

  const auto kMarkdown =
      formatter.FormatDaily(timeline_report, ReportFormat::kMarkdown);
  const auto kChineseMarkdown = formatter.FormatDailyLocalized(
      timeline_report, ReportFormat::kMarkdown, "zh");
  const auto kJapaneseMarkdown = formatter.FormatDailyLocalized(
      timeline_report, ReportFormat::kMarkdown, "ja");
  const auto kLatex =
      formatter.FormatDaily(timeline_report, ReportFormat::kLaTeX);
  const auto kTypst =
      formatter.FormatDaily(timeline_report, ReportFormat::kTyp);

  ExpectContains("English timeline heading", kMarkdown, "## Timeline",
                 failures);
  ExpectContains("Chinese timeline heading", kChineseMarkdown, "## 时间线",
                 failures);
  ExpectContains("Japanese timeline heading", kJapaneseMarkdown,
                 "## タイムライン", failures);

  constexpr std::string_view kPreciseActivity =
      "08:00:12 - 09:10:05 (1h 10m)";
  constexpr std::string_view kMinuteActivity = "09:20 - 10:20 (1h 0m)";
  ExpectContains("Markdown timeline seconds", kMarkdown, kPreciseActivity,
                 failures);
  ExpectContains("Markdown timeline zero seconds", kMarkdown, kMinuteActivity,
                 failures);
  ExpectNotContains("Markdown timeline strips zero seconds", kMarkdown,
                    "09:20:00 - 10:20:00", failures);
  ExpectContains("LaTeX timeline seconds", kLatex, kPreciseActivity, failures);
  ExpectContains("LaTeX timeline zero seconds", kLatex, kMinuteActivity,
                 failures);
  ExpectContains("Typst timeline seconds", kTypst, kPreciseActivity, failures);
  ExpectContains("Typst timeline zero seconds", kTypst, kMinuteActivity,
                 failures);
}

auto CheckDurationSecondsFormatting(int& failures) -> void {
  ExpectContains("duration seconds", TimeFormatDuration(3723), "1h 2m 3s",
                 failures);
  ExpectContains("duration zero seconds", TimeFormatDuration(3720), "1h 2m",
                 failures);
  ExpectNotContains("duration zero seconds omitted", TimeFormatDuration(3720),
                    "1h 2m 0s", failures);
  ExpectContains("average duration seconds", TimeFormatDuration(7446, 2),
                 "average: 1h 2m 3s/day", failures);
}

auto CheckMarkdownActivityRemarkLineBreaks(const ParityOutputs& outputs,
                                           int& failures) -> void {
  constexpr std::string_view kExpected =
      "  - **Activity Remark**:\n    Feature refactor<br>\n    Follow-up";
  ExpectContains("daily markdown multiline activity remark",
                 outputs.cli_by_format[0].day, kExpected, failures);
  ExpectContains("daily Android markdown multiline activity remark",
                 outputs.android_by_format[0].day, kExpected, failures);

  constexpr std::string_view kLatexExpected =
      "        \\item \\textbf{Activity Remark}:\\\\\n"
      "        Feature refactor\\\\\nFollow-up";
  ExpectContains("daily latex multiline activity remark",
                 outputs.cli_by_format[1].day, kLatexExpected, failures);
  ExpectContains("daily Android latex multiline activity remark",
                 outputs.android_by_format[1].day, kLatexExpected, failures);

  constexpr std::string_view kTypstExpected =
      "  + *Activity Remark:*\n    Feature refactor \\\n"
      "    Follow-up";
  ExpectContains("daily typst multiline activity remark",
                 outputs.cli_by_format[2].day, kTypstExpected, failures);
  ExpectContains("daily Android typst multiline activity remark",
                 outputs.android_by_format[2].day, kTypstExpected, failures);
}

auto CheckEndOnlyLocalizedMarkdown(infra_reports::ReportDtoFormatter& formatter,
                                   const DailyReportData& source_report,
                                   int& failures) -> void {
  DailyReportData end_only_report = source_report;
  end_only_report.total_duration = 0;
  end_only_report.activity_count = 1;
  end_only_report.detailed_records.clear();
  end_only_report.detailed_records.push_back(TimeRecord{
      .kind = ActivityRecordKind::kEndOnly,
      .end_time = "17:40:30",
      .project_path = "study_math",
      .duration_seconds = 0,
  });

  const auto kEnglishOutput =
      formatter.FormatDaily(end_only_report, ReportFormat::kMarkdown);
  const auto kChineseOutput = formatter.FormatDailyLocalized(
      end_only_report, ReportFormat::kMarkdown, "zh");
  const auto kJapaneseOutput = formatter.FormatDailyLocalized(
      end_only_report, ReportFormat::kMarkdown, "ja");
  ExpectContains("English end-only Markdown", kEnglishOutput,
                 "As of 17:40:30: study->math", failures);
  ExpectContains(
      "Chinese end-only Markdown",
      kChineseOutput,
      "截至 17:40:30: study->math", failures);
  ExpectContains(
      "Japanese end-only Markdown",
      kJapaneseOutput,
      "17:40:30 時点: study->math", failures);
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
      "2026-01", "2026-01-01", "2026-01-31", 31, 6, 5, 3, 2, 2);
  const WeeklyReportData weekly_report = BuildRangeFixture<WeeklyReportData>(
      "2026-W05", "2026-01-27", "2026-02-02", 7, 4, 3, 2, 1, 1);
  const YearlyReportData yearly_report = BuildRangeFixture<YearlyReportData>(
      "2026", "2026-01-01", "2026-12-31", 365, 120, 90, 60, 40, 30);
  const PeriodReportData range_report = BuildRangeFixture<PeriodReportData>(
      "Last 10 days", "2026-01-01", "2026-01-10", 10, 5, 4, 2, 1, 2);

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

    ExpectContains("Chinese localized daily report",
                   cli_formatter->FormatDailyLocalized(
                       daily_report, ReportFormat::kMarkdown, "zh"),
                   "每日报告", failures);
    ExpectContains("Japanese localized daily report",
                   cli_formatter->FormatDailyLocalized(
                       daily_report, ReportFormat::kMarkdown, "ja"),
                   "日次レポート", failures);
    ExpectContains("Unknown locale falls back to English",
                   cli_formatter->FormatDailyLocalized(
                       daily_report, ReportFormat::kMarkdown, "fr"),
                   "Daily Report for", failures);
    CheckDailyTimelineFormatting(*cli_formatter, daily_report, failures);
    CheckDurationSecondsFormatting(failures);
    CheckEndOnlyLocalizedMarkdown(*cli_formatter, daily_report, failures);
  } catch (const std::exception& exception) {
    std::cerr << "[FAIL] formatter setup threw exception: " << exception.what()
              << '\n';
    return 1;
  }

  RunMarkdownSnapshotCases(snapshot_root, outputs, update_snapshots, failures);
  RunLatexSnapshotCases(snapshot_root, outputs, update_snapshots, failures);
  RunTypstSnapshotCases(snapshot_root, outputs, update_snapshots, failures);
  CheckBooleanMetadataLabels(outputs, failures);
  CheckDailyActivityCountLabels(outputs, failures);
  CheckDailyStatusLabelsArePresent(outputs, failures);
  CheckMarkdownActivityRemarkLineBreaks(outputs, failures);

  if (failures == 0) {
    std::cout << "[PASS] time_tracker_formatter_parity_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] time_tracker_formatter_parity_tests failures: "
            << failures << '\n';
  return 1;
}

}  // namespace report_formatter_parity_internal
