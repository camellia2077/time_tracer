import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.reporting.data_querying;
import tracer.core.infrastructure.reporting.dto;
import tracer.core.infrastructure.reporting.exporting;
import tracer.core.infrastructure.reporting.querying;

#include "infra/tests/modules_smoke/reporting.hpp"
#include "infra/tests/modules_smoke/support.hpp"
#include "domain/reports/types/report_types.hpp"
#include "shared/types/reporting_errors.hpp"

auto RunInfrastructureModuleReportsSmoke() -> int {
  const auto kGetReportFormatDetails =
      &tracer::core::infrastructure::reports::GetReportFormatDetails;
  const auto kFormatDaily =
      &tracer::core::infrastructure::reports::ReportDtoFormatter::FormatDaily;
  const auto kRunPeriodQuery =
      &tracer::core::infrastructure::reports::ReportService::RunPeriodQuery;
  const auto kQueryAllDaily = &tracer::core::infrastructure::reports::
      SqliteReportDataQueryService::QueryAllDaily;
  const auto kListDailyTargets = &tracer::core::infrastructure::reports::
      SqliteReportDataQueryService::ListDailyTargets;
  const auto kListYearlyTargets =
      &tracer::core::infrastructure::reports::LazySqliteReportDataQueryService::
          ListYearlyTargets;
  (void)kGetReportFormatDetails;
  (void)kFormatDaily;
  (void)kRunPeriodQuery;
  (void)kQueryAllDaily;
  (void)kListDailyTargets;
  (void)kListYearlyTargets;

  const auto kMarkdownDetails =
      tracer::core::infrastructure::reports::GetReportFormatDetails(
          ReportFormat::kMarkdown);
  if (!kMarkdownDetails.has_value() ||
      kMarkdownDetails->dir_name != "markdown" ||
      kMarkdownDetails->extension != ".md") {
    return 31;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kDbSmokeDir =
      std::filesystem::path("temp") / "phase17_infra_module_reports";
  const std::filesystem::path kDbPath = kDbSmokeDir / "reports.sqlite";
  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  std::filesystem::create_directories(kDbSmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    tracer::core::infrastructure::reports::SqliteReportDataQueryService
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
      if (std::string_view(error.what()).find("Report target not found") ==
          std::string_view::npos) {
        return 39;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryMonthly("2024-12"));
      return 40;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Report target not found") ==
          std::string_view::npos) {
        return 41;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryWeekly("2024-W52"));
      return 42;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Report target not found") ==
          std::string_view::npos) {
        return 43;
      }
    }
    try {
      static_cast<void>(data_query_service.QueryYearly("2024"));
      return 44;
    } catch (const std::exception& error) {
      if (std::string_view(error.what()).find("Report target not found") ==
          std::string_view::npos) {
        return 45;
      }
    }

    tracer::core::infrastructure::reports::LazySqliteReportDataQueryService
        lazy_data_query_service(kDbPath,
                                std::make_shared<SmokePlatformClock>());
    if (!lazy_data_query_service.QueryAllDaily().empty()) {
      return 35;
    }
    if (!lazy_data_query_service.ListYearlyTargets().empty()) {
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
