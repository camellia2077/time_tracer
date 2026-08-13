import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.insights.data_querying;
import tracer.core.infrastructure.insights.dto;
import tracer.core.infrastructure.insights.exporting;
import tracer.core.infrastructure.insights.querying;

#include "infra/tests/modules_smoke/insights.hpp"
#include "infra/tests/modules_smoke/support.hpp"
#include "application/pipeline/importer/model/import_models.hpp"
#include "domain/insights/types/insights_types.hpp"
#include "shared/types/insights_errors.hpp"

#include <optional>

auto RunInfrastructureModuleInsightsSmoke() -> int {
  const auto kGetInsightsFormatDetails =
      &tracer::core::infrastructure::insights::GetInsightsFormatDetails;
  const auto kFormatDaily =
      &tracer::core::infrastructure::insights::InsightsDtoFormatter::FormatDaily;
  const auto kRunPeriodQuery =
      &tracer::core::infrastructure::insights::InsightsService::RunPeriodQuery;
  const auto kQueryAllDaily = &tracer::core::infrastructure::insights::
                                  SqliteInsightsDataQueryService::QueryAllDaily;
  const auto kListDailyTargets =
      &tracer::core::infrastructure::insights::SqliteInsightsDataQueryService::
          ListDailyTargets;
  const auto kListYearlyTargets =
      &tracer::core::infrastructure::insights::LazySqliteInsightsDataQueryService::
          ListYearlyTargets;
  (void)kGetInsightsFormatDetails;
  (void)kFormatDaily;
  (void)kRunPeriodQuery;
  (void)kQueryAllDaily;
  (void)kListDailyTargets;
  (void)kListYearlyTargets;

  const auto kMarkdownDetails =
      tracer::core::infrastructure::insights::GetInsightsFormatDetails(
          InsightsFormat::kMarkdown);
  if (!kMarkdownDetails.has_value() ||
      kMarkdownDetails->dir_name != "markdown" ||
      kMarkdownDetails->extension != ".md") {
    return 31;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kDbSmokeDir =
      std::filesystem::path("temp") / "phase17_infra_module_insights";
  const std::filesystem::path kDbPath = kDbSmokeDir / "insights.sqlite";
  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  std::filesystem::create_directories(kDbSmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    tracer::core::infrastructure::insights::SqliteInsightsDataQueryService
        data_query_service(connection.GetDb(),
                           std::make_shared<SmokePlatformClock>());
    if (!data_query_service.QueryAllDaily().empty()) {
      return 32;
    }
    if (!data_query_service.ListDailyTargets().empty()) {
      return 33;
    }
    if (!data_query_service.QueryPeriodBatch({}).empty()) {
      return 34;
    }

    tracer::core::infrastructure::persistence::importer::sqlite::Statement
        statement(connection.GetDb());
    tracer::core::infrastructure::persistence::importer::sqlite::Writer writer(
        connection.GetDb(), statement.GetInsertDayStmt(),
        statement.GetInsertRecordStmt(), statement.GetInsertProjectStmt());
    writer.InsertDays({DayData{.date = "2023-11-30",
                               .remark = "",
                               .getup_time = std::nullopt,
                               .year = 2023,
                               .month = 11,
                               .activity_count = 1}});
    writer.InsertRecords(
        {TimeRecordInternal{.kind = ActivityRecordKind::kEndOnly,
                            .logical_id = 9,
                            .start_timestamp = 0,
                            .end_timestamp = 1770000000,
                            .start_time_str = "",
                            .end_time_str = "12:00",
                            .project_path = "work",
                            .duration_seconds = 0,
                            .remark = std::nullopt,
                            .date = "2023-11-30"}});
    const auto kEndOnlyDay = data_query_service.QueryDaily("2023-11-30");
    if (kEndOnlyDay.activity_count != 1 || kEndOnlyDay.total_duration != 0 ||
        kEndOnlyDay.detailed_records.size() != 1U ||
        kEndOnlyDay.detailed_records.front().kind !=
            ActivityRecordKind::kEndOnly) {
      return 48;
    }
    const auto kEndOnlyRange =
        data_query_service.QueryRange("2023-11-30", "2023-11-30");
    if (!kEndOnlyRange.has_records || kEndOnlyRange.activity_days.size() != 1U ||
        kEndOnlyRange.activity_days.front().total_duration != 0 ||
        kEndOnlyRange.activity_days.front().detailed_records.size() != 1U ||
        kEndOnlyRange.activity_days.front().detailed_records.front().kind !=
            ActivityRecordKind::kEndOnly) {
      return 49;
    }

    {
      const auto kEmptyRecent = data_query_service.QueryPeriod(7);
      if (kEmptyRecent.has_records || kEmptyRecent.matched_day_count != 0 ||
          kEmptyRecent.matched_record_count != 0) {
        return 46;
      }
    }
    {
      const auto kEmptyRange =
          data_query_service.QueryRange("2024-12-01", "2024-12-31");
      if (kEmptyRange.has_records || kEmptyRange.matched_day_count != 0 ||
          kEmptyRange.matched_record_count != 0) {
        return 47;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryDaily("2024-12-31"));
      return 38;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Insights target not found") ==
          std::string_view::npos) {
        return 39;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryMonthly("2024-12"));
      return 40;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Insights target not found") ==
          std::string_view::npos) {
        return 41;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryWeekly("2024-W52"));
      return 42;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Insights target not found") ==
          std::string_view::npos) {
        return 43;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryYearly("2024"));
      return 44;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Insights target not found") ==
          std::string_view::npos) {
        return 45;
      }
    }

    tracer::core::infrastructure::insights::LazySqliteInsightsDataQueryService
        lazy_data_query_service(kDbPath,
                                std::make_shared<SmokePlatformClock>());
    if (lazy_data_query_service.QueryAllDaily().size() != 1U) {
      return 35;
    }
    const auto kLazyYears = lazy_data_query_service.ListYearlyTargets();
    if (kLazyYears.size() != 1U || kLazyYears.front() != "2023") {
      return 36;
    }
  } catch (const std::exception& error) {
    return 37;
  } catch (...) {
    return 37;
  }

  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  return 0;
}
