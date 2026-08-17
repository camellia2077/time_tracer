import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.query.data.renderers;
import tracer.core.infrastructure.query.data.repository;
import tracer.core.infrastructure.query.data.stats;

#include <filesystem>
#include <optional>

#include "application/pipeline/importer/model/import_models.hpp"
#include "application/dto/query_requests.hpp"
#include "domain/insights/models/activity_aggregate.hpp"
#include "domain/insights/models/project_tree.hpp"
#include "infra/tests/modules_smoke/query.hpp"

auto RunInfrastructureModuleQueryStatsRepositorySmoke() -> int {
  const auto kComputeDayDurationStats =
      &tracer::core::infrastructure::query::data::stats::
          ComputeDayDurationStats;
  const auto kBuildInsightsChartSeries =
      &tracer::core::infrastructure::query::data::stats::
          BuildInsightsChartSeries;
  const auto kBuildInsightsCompositionStats =
      &tracer::core::infrastructure::query::data::stats::
          BuildInsightsCompositionStats;
  const auto kResolveAverageDenominator =
      &tracer::core::infrastructure::query::data::stats::
          ResolveAverageDenominator;
  const auto kCalculateAverageOrZero =
      &tracer::core::infrastructure::query::data::stats::CalculateAverageOrZero<
          long long>;
  const auto kStatsBoundaryReady =
      &tracer::core::infrastructure::query::data::stats::BoundaryReady;
  const auto kQueryYears =
      &tracer::core::infrastructure::query::data::QueryYears;
  const auto kQueryProjectTree =
      &tracer::core::infrastructure::query::data::QueryProjectTree;
  const auto kRenderListOutput =
      &tracer::core::infrastructure::query::data::renderers::RenderListOutput;
  const auto kRenderDayDurationsOutput =
      &tracer::core::infrastructure::query::data::renderers::
          RenderDayDurationsOutput;
  const auto kRenderersBoundaryReady =
      &tracer::core::infrastructure::query::data::renderers::BoundaryReady;
  (void)kComputeDayDurationStats;
  (void)kBuildInsightsChartSeries;
  (void)kBuildInsightsCompositionStats;
  (void)kResolveAverageDenominator;
  (void)kCalculateAverageOrZero;
  (void)kStatsBoundaryReady;
  (void)kQueryYears;
  (void)kQueryProjectTree;
  (void)kRenderListOutput;
  (void)kRenderDayDurationsOutput;
  (void)kRenderersBoundaryReady;

  if (!tracer::core::infrastructure::query::data::stats::BoundaryReady()) {
    return 5;
  }
  if (!tracer::core::infrastructure::query::data::renderers::BoundaryReady()) {
    return 6;
  }

  const std::vector<tracer::core::infrastructure::query::data::DayDurationRow>
      kRows = {
          {.date = "2026-02-01", .total_seconds = 3600, .record_count = 1},
          {.date = "2026-02-02", .total_seconds = 0, .record_count = 1},
          {.date = "2026-02-03", .total_seconds = 7200, .record_count = 1},
      };
  const auto kStats =
      tracer::core::infrastructure::query::data::stats::ComputeDayDurationStats(
          kRows);
  if (kStats.count != 3 || kStats.min_seconds != 0.0 ||
      kStats.max_seconds != 7200.0) {
    return 7;
  }

  const auto kSeries = tracer::core::infrastructure::query::data::stats::
      BuildInsightsChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-03"}, kRows);
  if (kSeries.series.size() != 3U ||
      kSeries.series.front().date != "2026-02-01" ||
      kSeries.series.back().date != "2026-02-03" ||
      kSeries.stats.active_days != 3 ||
      kSeries.stats.activity.total_duration_seconds != 10800) {
    return 8;
  }
  if (kSeries.stats.average_denominator_days != 3 ||
      kSeries.stats.average_duration_seconds != 3600 ||
      kSeries.stats.activity.occurrence_count != 3 ||
      kSeries.stats.average_duration_per_occurrence_seconds != 3600) {
    return 18;
  }

  const std::vector<tracer::core::infrastructure::query::data::DayDurationRow>
      kDistributionRows = {
          {.date = "2026-02-01", .total_seconds = 3600, .record_count = 1},
          {.date = "2026-02-03", .total_seconds = 3600, .record_count = 1},
      };
  const auto kDistributionSeries = tracer::core::infrastructure::query::data::
      stats::BuildInsightsChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-05"},
          kDistributionRows);
  if (!kDistributionSeries.stats.mode_duration_seconds.has_value() ||
      *kDistributionSeries.stats.mode_duration_seconds != 3600.0 ||
      kDistributionSeries.stats.median_duration_seconds != 3600.0) {
    return 22;
  }

  const auto kCalendarSeries = tracer::core::infrastructure::query::data::
      stats::BuildInsightsChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-04"}, kRows,
          tracer_core::core::dto::InsightsAverageDayBasis::kCalendarDays);
  if (kCalendarSeries.stats.average_denominator_days != 4 ||
      kCalendarSeries.stats.average_duration_seconds != 2700) {
    return 19;
  }

  const auto kEmptySeries = tracer::core::infrastructure::query::data::stats::
      BuildInsightsChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-03"}, {});
  if (kEmptySeries.series.size() != 3U || kEmptySeries.stats.active_days != 0 ||
      kEmptySeries.stats.activity.total_duration_seconds != 0 ||
      kEmptySeries.stats.mode_duration_seconds.has_value() ||
      kEmptySeries.stats.minimum_duration_seconds != 0.0 ||
      kEmptySeries.stats.maximum_duration_seconds != 0.0) {
    return 23;
  }

  const std::vector<tracer::core::infrastructure::query::data::DayDurationRow>
      kTieRows = {
          {.date = "2026-02-01", .total_seconds = 100, .record_count = 1},
          {.date = "2026-02-02", .total_seconds = 200, .record_count = 1},
          {.date = "2026-02-03", .total_seconds = 200, .record_count = 1},
          {.date = "2026-02-04", .total_seconds = 100, .record_count = 1},
      };
  const auto kTieSeries = tracer::core::infrastructure::query::data::stats::
      BuildInsightsChartSeries(
          {.start_date = "2026-02-01", .end_date = "2026-02-04"}, kTieRows);
  if (!kTieSeries.stats.mode_duration_seconds.has_value() ||
      *kTieSeries.stats.mode_duration_seconds != 100.0 ||
      kTieSeries.stats.median_duration_seconds != 150.0) {
    return 24;
  }

  ActivityAggregate aggregate;
  aggregate.AddDuration(3600);
  if (aggregate.total_duration_seconds != 3600 ||
      aggregate.occurrence_count != 0) {
    return 25;
  }
  aggregate.Add(1800, 2);
  if (aggregate.total_duration_seconds != 5400 ||
      aggregate.occurrence_count != 2) {
    return 26;
  }

  insights::ProjectTree kTree;
  kTree.emplace("study", insights::ProjectNode{
                             .duration = 9000,
                             .occurrence_count = 3,
                             .children = {},
                         });
  const auto kCompositionStats = tracer::core::infrastructure::query::data::
      stats::BuildInsightsCompositionStats(
          kTree, {kRows[0], kRows[2]},
          tracer_core::core::dto::InsightsAverageDayBasis::kActiveDays, 4);
  const auto kCompositionNode = kCompositionStats.nodes.find("study");
  if (kCompositionStats.average_denominator_days != 2 ||
      kCompositionNode == kCompositionStats.nodes.end() ||
      kCompositionNode->second.average_duration_seconds != 4500 ||
      kCompositionNode->second.average_duration_per_occurrence_seconds !=
          3000) {
    return 20;
  }
  if (kResolveAverageDenominator(
          tracer_core::core::dto::InsightsAverageDayBasis::kCalendarDays, 2,
          4) != 4 ||
      kCalculateAverageOrZero(9000, 2) != 4500) {
    return 21;
  }

  const auto kSemanticOutput =
      tracer::core::infrastructure::query::data::renderers::RenderListOutput(
          "years", {"2026"},
          tracer_core::core::dto::DataQueryOutputMode::kSemanticJson);
  if (kSemanticOutput.find("\"output_mode\":\"semantic_json\"") ==
      std::string::npos) {
    return 9;
  }

  const auto kTextOutput = tracer::core::infrastructure::query::data::
      renderers::RenderDayDurationsOutput(
          "days_duration", kRows,
          tracer_core::core::dto::DataQueryOutputMode::kText);
  if (kTextOutput.find("2026-02-01") == std::string::npos ||
      kTextOutput.find("Total: 3") == std::string::npos) {
    return 10;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kQuerySmokeDir =
      std::filesystem::path("temp") / "phase13_infra_module_smoke";
  const std::filesystem::path kDbPath = kQuerySmokeDir / "query.sqlite";
  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  std::filesystem::create_directories(kQuerySmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    tracer::core::infrastructure::query::data::QueryFilters filters{};

    if (tracer::core::infrastructure::query::data::QueryLatestTrackedDate(
            connection.GetDb())
            .has_value()) {
      return 11;
    }
    if (!tracer::core::infrastructure::query::data::QueryProjectRootNames(
             connection.GetDb())
             .empty()) {
      return 12;
    }
    if (!tracer::core::infrastructure::query::data::QueryDayDurations(
             connection.GetDb(), filters)
             .empty()) {
      return 13;
    }
    if (!tracer::core::infrastructure::query::data::QueryDays(
             connection.GetDb(), filters.kYear, filters.kMonth,
             filters.from_date, filters.to_date, filters.reverse, filters.limit)
             .empty()) {
      return 14;
    }
    if (!tracer::core::infrastructure::query::data::QueryProjectTree(
             connection.GetDb(), filters)
             .empty() ||
        !tracer::core::infrastructure::query::data::QueryYears(
             connection.GetDb())
             .empty()) {
      return 15;
    }

    tracer::core::infrastructure::persistence::importer::sqlite::Statement
        statement(connection.GetDb());
    tracer::core::infrastructure::persistence::importer::sqlite::Writer writer(
        connection.GetDb(), statement.GetInsertDayStmt(),
        statement.GetInsertRecordStmt(), statement.GetInsertProjectStmt());
    writer.InsertDays({DayData{.date = "2026-02-02",
                               .remark = "",
                               .getup_time = std::nullopt,
                               .year = 2026,
                               .month = 2,
                               .activity_count = 1}});
    writer.InsertRecords(
        {TimeRecordInternal{.kind = ActivityRecordKind::kEndOnly,
                            .logical_id = 7,
                            .start_timestamp = 0,
                            .end_timestamp = 1770000000,
                            .start_time_str = "",
                            .end_time_str = "12:00:00",
                            .project_path = "work",
                            .duration_seconds = 0,
                            .remark = std::nullopt,
                            .date = "2026-02-02"}});

    const auto kPersistedRows =
        tracer::core::infrastructure::query::data::QueryDayDurations(
            connection.GetDb(), filters);
    if (kPersistedRows.size() != 1U ||
        kPersistedRows.front().total_seconds != 0 ||
        kPersistedRows.front().record_count != 1) {
      return 16;
    }
    const auto kRootRows = tracer::core::infrastructure::query::data::
        QueryDayDurationsByRootInDateRange(connection.GetDb(), "work",
                                           "2026-02-02", "2026-02-02");
    if (kRootRows.size() != 1U || kRootRows.front().total_seconds != 0 ||
        kRootRows.front().record_count != 1) {
      return 17;
    }
  } catch (...) {
    return 11;
  }

  std::filesystem::remove_all(kQuerySmokeDir, cleanup_error);
  return 0;
}
