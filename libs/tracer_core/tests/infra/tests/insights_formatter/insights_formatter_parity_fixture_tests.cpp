// infrastructure/tests/insights_formatter/insights_formatter_parity_fixture_tests.cpp
import tracer.core.infrastructure.config.loader;
import tracer.core.domain.insights.models.daily_insights_data;
import tracer.core.domain.insights.models.period_insights_models;
import tracer.core.domain.insights.models.project_tree;
import tracer.core.domain.insights.types.insights_types;
import tracer.core.infrastructure.insights.dto;

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>

#include "application/ports/insights/i_insights_formatter_registry.hpp"
#include "domain/model/time_data_models.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/insights/facade/android_static_insights_formatter_registrar.hpp"
#include "infra/insights/shared/utils/format/insights_string_utils.hpp"
#include "infra/insights/shared/utils/format/time_format.hpp"
#include "infra/tests/insights_formatter/insights_formatter_parity_internal.hpp"

namespace {

// Test fixture code intentionally favors explicit literals and sample labels.
// NOLINTBEGIN(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

namespace fs = std::filesystem;
namespace infra_config = tracer::core::infrastructure::config;
namespace infra_insights = tracer::core::infrastructure::insights;
namespace insights = tracer::core::domain::modinsights;
using insights_formatter_parity_internal::CaseOutputs;
using insights_formatter_parity_internal::ParityOutputs;
using tracer::core::domain::modinsights::DailyInsightsData;
using tracer::core::domain::modinsights::MonthlyInsightsData;
using tracer::core::domain::modinsights::PeriodInsightsData;
using tracer::core::domain::modinsights::InsightsFormat;
using tracer::core::domain::modinsights::TimeRecord;
using tracer::core::domain::modinsights::WeeklyInsightsData;
using tracer::core::domain::modinsights::YearlyInsightsData;

enum class FormatterPipeline { kDefaultRegistry, kAndroidStatic };

auto BuildRepoRoot() -> fs::path {
  return fs::path(__FILE__)
      .parent_path()   // insights_formatter
      .parent_path()   // tests
      .parent_path()   // infrastructure
      .parent_path()   // tests
      .parent_path()   // tracer_core
      .parent_path()   // libs
      .parent_path();  // repo root
}

auto BuildInsightsCatalog(const fs::path& repo_root) -> InsightsCatalog {
  InsightsCatalog catalog;

  const fs::path insights_config_root =
      repo_root / "config" / "program" / "insights";
  const fs::path markdown_config_dir = insights_config_root / "markdown";
  const fs::path markdown_default_config_dir = markdown_config_dir / "en";
  const fs::path latex_config_dir = insights_config_root / "latex";
  const fs::path typst_config_dir = insights_config_root / "typst";

  catalog.loaded_insights.markdown.day =
      infra_config::InsightsConfigLoader::LoadDailyMdConfig(
          markdown_default_config_dir / "day.toml");
  catalog.loaded_insights.markdown.month =
      infra_config::InsightsConfigLoader::LoadMonthlyMdConfig(
          markdown_default_config_dir / "month.toml");
  catalog.loaded_insights.markdown.period =
      infra_config::InsightsConfigLoader::LoadPeriodMdConfig(
          markdown_default_config_dir / "period.toml");
  catalog.loaded_insights.markdown.week =
      infra_config::InsightsConfigLoader::LoadWeeklyMdConfig(
          markdown_default_config_dir / "week.toml");
  catalog.loaded_insights.markdown.year =
      infra_config::InsightsConfigLoader::LoadYearlyMdConfig(
          markdown_default_config_dir / "year.toml");

  for (const std::string_view locale : {"en", "zh", "ja"}) {
    const fs::path locale_dir = markdown_config_dir / std::string(locale);
    MarkdownInsightsConfigs localized;
    localized.day = infra_config::InsightsConfigLoader::LoadDailyMdConfig(
        locale_dir / "day.toml");
    localized.month = infra_config::InsightsConfigLoader::LoadMonthlyMdConfig(
        locale_dir / "month.toml");
    localized.period = infra_config::InsightsConfigLoader::LoadPeriodMdConfig(
        locale_dir / "period.toml");
    localized.week = infra_config::InsightsConfigLoader::LoadWeeklyMdConfig(
        locale_dir / "week.toml");
    localized.year = infra_config::InsightsConfigLoader::LoadYearlyMdConfig(
        locale_dir / "year.toml");
    catalog.loaded_insights.markdown_locales.emplace(std::string(locale),
                                                    std::move(localized));
  }

  catalog.loaded_insights.latex.day =
      infra_config::InsightsConfigLoader::LoadDailyTexConfig(latex_config_dir /
                                                           "day.toml");
  catalog.loaded_insights.latex.month =
      infra_config::InsightsConfigLoader::LoadMonthlyTexConfig(latex_config_dir /
                                                             "month.toml");
  catalog.loaded_insights.latex.period =
      infra_config::InsightsConfigLoader::LoadPeriodTexConfig(latex_config_dir /
                                                            "period.toml");
  catalog.loaded_insights.latex.week =
      infra_config::InsightsConfigLoader::LoadWeeklyTexConfig(latex_config_dir /
                                                            "week.toml");
  catalog.loaded_insights.latex.year =
      infra_config::InsightsConfigLoader::LoadYearlyTexConfig(latex_config_dir /
                                                            "year.toml");

  catalog.loaded_insights.typst.day =
      infra_config::InsightsConfigLoader::LoadDailyTypConfig(typst_config_dir /
                                                           "day.toml");
  catalog.loaded_insights.typst.month =
      infra_config::InsightsConfigLoader::LoadMonthlyTypConfig(typst_config_dir /
                                                             "month.toml");
  catalog.loaded_insights.typst.period =
      infra_config::InsightsConfigLoader::LoadPeriodTypConfig(typst_config_dir /
                                                            "period.toml");
  catalog.loaded_insights.typst.week =
      infra_config::InsightsConfigLoader::LoadWeeklyTypConfig(typst_config_dir /
                                                            "week.toml");
  catalog.loaded_insights.typst.year =
      infra_config::InsightsConfigLoader::LoadYearlyTypConfig(typst_config_dir /
                                                            "year.toml");

  return catalog;
}

auto BuildFormatter(FormatterPipeline pipeline, const InsightsCatalog& catalog)
    -> std::unique_ptr<infra_insights::InsightsDtoFormatter> {
  if (pipeline == FormatterPipeline::kDefaultRegistry) {
    auto registry =
        tracer_core::application::ports::CreateInsightsFormatterRegistry();
    registry->RegisterFormatters();
  } else {
    auto static_registrar = std::make_shared<
        infrastructure::insights::AndroidStaticInsightsFormatterRegistrar>(
        infrastructure::insights::AndroidStaticInsightsFormatterPolicy::
            AllFormats());
    auto registry =
        tracer_core::application::ports::CreateInsightsFormatterRegistry(
            static_registrar);
    registry->RegisterFormatters();
  }

  return std::make_unique<infra_insights::InsightsDtoFormatter>(catalog);
}

auto BuildDailyProjectTree() -> insights::ProjectTree {
  insights::ProjectNode work{.duration = 7800, .occurrence_count = 2};
  work.children["Coding"] =
      insights::ProjectNode{.duration = 4200, .occurrence_count = 1};
  work.children["Review"] =
      insights::ProjectNode{.duration = 3600, .occurrence_count = 1};

  insights::ProjectNode life{.duration = 4800, .occurrence_count = 2};
  life.children["Reading"] =
      insights::ProjectNode{.duration = 3000, .occurrence_count = 1};
  life.children["Exercise"] =
      insights::ProjectNode{.duration = 1800, .occurrence_count = 1};

  insights::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildRangeProjectTree() -> insights::ProjectTree {
  insights::ProjectNode work{.duration = 28800, .occurrence_count = 5};
  work.children["Coding"] =
      insights::ProjectNode{.duration = 18000, .occurrence_count = 3};
  work.children["Review"] =
      insights::ProjectNode{.duration = 10800, .occurrence_count = 2};

  insights::ProjectNode life{.duration = 25200, .occurrence_count = 4};
  life.children["Reading"] =
      insights::ProjectNode{.duration = 18000, .occurrence_count = 3};
  life.children["Exercise"] =
      insights::ProjectNode{.duration = 7200, .occurrence_count = 1};

  insights::ProjectTree tree;
  tree["Work"] = work;
  tree["Life"] = life;
  return tree;
}

auto BuildDailyFixture() -> DailyInsightsData {
  DailyInsightsData insights;
  insights.date = "2021-01-03";
  insights.metadata.statuses = {
      {.id = "study", .label = "Study", .occurrence_count = 2,
       .total_duration = 7200},
      {.id = "exercise", .label = "Exercise", .occurrence_count = 1,
       .total_duration = 1800},
  };
  insights.metadata.getup_time = "07:30:00";
  insights.metadata.remark = "Deep work\nEvening workout";
  insights.total_duration = 12600;
  insights.activity_count = 3;
  insights.stats["sleep_total_time"] = 25200;
  insights.stats["study_time"] = 3600;
  insights.stats["total_exercise_time"] = 1800;
  insights.stats["anaerobic_time"] = 900;
  insights.stats["cardio_time"] = 900;
  insights.stats["grooming_time"] = 1200;
  insights.stats["recreation_time"] = 2400;
  insights.stats["recreation_zhihu_time"] = 600;
  insights.stats["recreation_bilibili_time"] = 1200;
  insights.stats["recreation_douyin_time"] = 600;
  insights.detailed_records.push_back(TimeRecord{
      .start_time = "08:00:00",
      .end_time = "09:10:00",
      .project_path = "Work_Coding",
      .duration_seconds = 4200,
      .activityRemark =
          std::optional<std::string>{"Feature refactor\nFollow-up"},
  });
  insights.detailed_records.push_back(TimeRecord{
      .start_time = "09:20:00",
      .end_time = "10:20:00",
      .project_path = "Work_Review",
      .duration_seconds = 3600,
      .activityRemark = std::nullopt,
  });
  insights.detailed_records.push_back(TimeRecord{
      .start_time = "20:00:00",
      .end_time = "20:30:00",
      .project_path = "Life_Exercise",
      .duration_seconds = 1800,
      .activityRemark = std::optional<std::string>{"Cardio"},
  });
  insights.project_tree = BuildDailyProjectTree();
  return insights;
}

template <typename RangeInsightsType>
auto BuildRangeFixture(const std::string& range_label,
                       const std::string& start_date,
                       const std::string& end_date, int requested_days,
                       int actual_days, int status_days, int exercise_days,
                       int cardio_days, int anaerobic_days) -> RangeInsightsType {
  RangeInsightsType insights;
  insights.range_label = range_label;
  insights.start_date = start_date;
  insights.end_date = end_date;
  insights.requested_days = requested_days;
  insights.total_duration = 54000;
  insights.actual_days = actual_days;
  insights.matched_record_count = actual_days * 2;
  insights.status_true_days = status_days;
  insights.exercise_true_days = exercise_days;
  insights.cardio_true_days = cardio_days;
  insights.anaerobic_true_days = anaerobic_days;
  insights.statuses = {
      {.id = "status", .label = "Status Days", .occurrence_count = status_days,
       .total_duration = static_cast<std::int64_t>(status_days) * 3600},
      {.id = "exercise", .label = "Exercise Days", .occurrence_count = exercise_days,
       .total_duration = static_cast<std::int64_t>(exercise_days) * 3600},
      {.id = "cardio", .label = "Cardio Days", .occurrence_count = cardio_days,
       .total_duration = static_cast<std::int64_t>(cardio_days) * 3600},
      {.id = "anaerobic", .label = "Anaerobic Days", .occurrence_count = anaerobic_days,
       .total_duration = static_cast<std::int64_t>(anaerobic_days) * 3600},
  };
  insights.is_valid = true;
  insights.project_tree = BuildRangeProjectTree();
  return insights;
}

auto CollectOutputs(infra_insights::InsightsDtoFormatter& formatter,
                    const DailyInsightsData& daily_insights,
                    const MonthlyInsightsData& monthly_insights,
                    const WeeklyInsightsData& weekly_insights,
                    const YearlyInsightsData& yearly_insights,
                    const PeriodInsightsData& range_insights, InsightsFormat format)
    -> CaseOutputs {
  CaseOutputs outputs;
  outputs.day = formatter.FormatDaily(daily_insights, format);
  outputs.month = formatter.FormatMonthly(monthly_insights, format);
  outputs.week = formatter.FormatWeekly(weekly_insights, format);
  outputs.year = formatter.FormatYearly(yearly_insights, format);
  outputs.range = formatter.FormatPeriod(range_insights, format);
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
  std::cerr << "[FAIL] " << case_name << " should not contain `" << unexpected
            << "`.\n";
}

auto CheckBooleanMetadataLabels(const ParityOutputs& outputs, int& failures)
    -> void {
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

auto CheckDailyActivityCountLabels(const ParityOutputs& outputs, int& failures)
    -> void {
  constexpr std::string_view kMarkdownExpected = "- **Activity Count**: 3";
  constexpr std::string_view kLatexExpected = "\\textbf{Activity Count}: 3";
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
  constexpr std::string_view kMarkdownStudy = "- **Study**: 2 times (2h 0m)";
  constexpr std::string_view kMarkdownExercise = "- **Exercise**: 1 times (0h 30m)";
  constexpr std::string_view kLatexStudy = "\\textbf{Study}: 2 times (2h 0m)";
  constexpr std::string_view kLatexExercise = "\\textbf{Exercise}: 1 times (0h 30m)";
  constexpr std::string_view kTypstStudy = "+ *Study:* 2 times (2h 0m)";
  constexpr std::string_view kTypstExercise = "+ *Exercise:* 1 times (0h 30m)";

  ExpectContains("CLI daily Markdown Study status",
                 outputs.cli_by_format[0].day, kMarkdownStudy, failures);
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
                 outputs.android_by_format[0].day, kMarkdownExercise, failures);
  ExpectContains("Android daily LaTeX Study status",
                 outputs.android_by_format[1].day, kLatexStudy, failures);
  ExpectContains("Android daily LaTeX Exercise status",
                 outputs.android_by_format[1].day, kLatexExercise, failures);
  ExpectContains("Android daily Typst Study status",
                 outputs.android_by_format[2].day, kTypstStudy, failures);
  ExpectContains("Android daily Typst Exercise status",
                 outputs.android_by_format[2].day, kTypstExercise, failures);
}

auto CheckDailyTimelineFormatting(infra_insights::InsightsDtoFormatter& formatter,
                                  const DailyInsightsData& source_insights,
                                  int& failures) -> void {
  DailyInsightsData timeline_insights = source_insights;
  timeline_insights.activity_count = 2;
  timeline_insights.detailed_records = {
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
      formatter.FormatDaily(timeline_insights, InsightsFormat::kMarkdown);
  const auto kChineseMarkdown = formatter.FormatDailyLocalized(
      timeline_insights, InsightsFormat::kMarkdown, "zh");
  const auto kJapaneseMarkdown = formatter.FormatDailyLocalized(
      timeline_insights, InsightsFormat::kMarkdown, "ja");
  const auto kLatex =
      formatter.FormatDaily(timeline_insights, InsightsFormat::kLaTeX);
  const auto kTypst =
      formatter.FormatDaily(timeline_insights, InsightsFormat::kTyp);

  ExpectContains("English timeline heading", kMarkdown, "## Timeline",
                 failures);
  ExpectContains("Chinese timeline heading", kChineseMarkdown, "## 时间线",
                 failures);
  ExpectContains("Japanese timeline heading", kJapaneseMarkdown,
                 "## タイムライン", failures);

  constexpr std::string_view kPreciseActivity = "08:00:12 - 09:10:05 (1h 10m)";
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

auto CheckEndOnlyLocalizedMarkdown(infra_insights::InsightsDtoFormatter& formatter,
                                   const DailyInsightsData& source_insights,
                                   int& failures) -> void {
  DailyInsightsData end_only_insights = source_insights;
  end_only_insights.total_duration = 0;
  end_only_insights.activity_count = 1;
  end_only_insights.detailed_records.clear();
  end_only_insights.detailed_records.push_back(TimeRecord{
      .kind = ActivityRecordKind::kEndOnly,
      .end_time = "17:40:30",
      .project_path = "study_math",
      .duration_seconds = 0,
  });

  const auto kEnglishOutput =
      formatter.FormatDaily(end_only_insights, InsightsFormat::kMarkdown);
  const auto kChineseOutput = formatter.FormatDailyLocalized(
      end_only_insights, InsightsFormat::kMarkdown, "zh");
  const auto kJapaneseOutput = formatter.FormatDailyLocalized(
      end_only_insights, InsightsFormat::kMarkdown, "ja");
  ExpectContains("English end-only Markdown", kEnglishOutput,
                 "As of 17:40:30: study->math", failures);
  ExpectContains("Chinese end-only Markdown", kChineseOutput,
                 "截至 17:40:30: study->math", failures);
  ExpectContains("Japanese end-only Markdown", kJapaneseOutput,
                 "17:40:30 時点: study->math", failures);
}

// NOLINTEND(readability-magic-numbers,readability-identifier-naming,bugprone-easily-swappable-parameters,modernize-use-auto,modernize-use-designated-initializers)

}  // namespace

namespace insights_formatter_parity_internal {

auto RunFormatterParityTests() -> int {
  int failures = 0;

  const fs::path repo_root = BuildRepoRoot();
  const fs::path snapshot_root =
      repo_root / "test" / "golden" / "insights_formatter_parity" / "v1";
  const bool update_snapshots =
      std::getenv("TT_UPDATE_FORMATTER_SNAPSHOTS") != nullptr;

  InsightsCatalog catalog;
  try {
    catalog = BuildInsightsCatalog(repo_root);
  } catch (const std::exception& exception) {
    std::cerr << "[FAIL] Failed to load insights config for parity tests: "
              << exception.what() << '\n';
    return 1;
  }

  const DailyInsightsData daily_insights = BuildDailyFixture();
  const MonthlyInsightsData monthly_insights = BuildRangeFixture<MonthlyInsightsData>(
      "2026-01", "2026-01-01", "2026-01-31", 31, 6, 5, 3, 2, 2);
  const WeeklyInsightsData weekly_insights = BuildRangeFixture<WeeklyInsightsData>(
      "2026-W05", "2026-01-27", "2026-02-02", 7, 4, 3, 2, 1, 1);
  const YearlyInsightsData yearly_insights = BuildRangeFixture<YearlyInsightsData>(
      "2026", "2026-01-01", "2026-12-31", 365, 120, 90, 60, 40, 30);
  const PeriodInsightsData range_insights = BuildRangeFixture<PeriodInsightsData>(
      "Last 10 days", "2026-01-01", "2026-01-10", 10, 5, 4, 2, 1, 2);

  ParityOutputs outputs;
  try {
    auto cli_formatter =
        BuildFormatter(FormatterPipeline::kDefaultRegistry, catalog);
    outputs.cli_by_format[0] = CollectOutputs(
        *cli_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kMarkdown);
    outputs.cli_by_format[1] = CollectOutputs(
        *cli_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kLaTeX);
    outputs.cli_by_format[2] = CollectOutputs(
        *cli_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kTyp);

    auto android_formatter =
        BuildFormatter(FormatterPipeline::kAndroidStatic, catalog);
    outputs.android_by_format[0] = CollectOutputs(
        *android_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kMarkdown);
    outputs.android_by_format[1] = CollectOutputs(
        *android_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kLaTeX);
    outputs.android_by_format[2] = CollectOutputs(
        *android_formatter, daily_insights, monthly_insights, weekly_insights,
        yearly_insights, range_insights, InsightsFormat::kTyp);

    ExpectContains("Chinese localized daily insights",
                   cli_formatter->FormatDailyLocalized(
                       daily_insights, InsightsFormat::kMarkdown, "zh"),
                   "摘要", failures);
    ExpectContains("Japanese localized daily insights",
                   cli_formatter->FormatDailyLocalized(
                       daily_insights, InsightsFormat::kMarkdown, "ja"),
                   "サマリー", failures);
    ExpectContains("Unknown locale falls back to English",
                   cli_formatter->FormatDailyLocalized(
                       daily_insights, InsightsFormat::kMarkdown, "fr"),
                   "Summary", failures);
    CheckDailyTimelineFormatting(*cli_formatter, daily_insights, failures);
    CheckDurationSecondsFormatting(failures);
    CheckEndOnlyLocalizedMarkdown(*cli_formatter, daily_insights, failures);
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

}  // namespace insights_formatter_parity_internal
