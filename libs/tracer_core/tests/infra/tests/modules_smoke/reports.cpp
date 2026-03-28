import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.reporting.data_querying;
import tracer.core.infrastructure.reporting.dto;
import tracer.core.infrastructure.reporting.exporting;
import tracer.core.infrastructure.reporting.querying;

#include "infra/config/models/report_catalog.hpp"
#include "infra/tests/modules_smoke/reporting.hpp"
#include "infra/tests/modules_smoke/support.hpp"
#include "domain/reports/types/report_types.hpp"

namespace {

auto BuildMinimalReportCatalog() -> ReportCatalog {
  ReportCatalog catalog;
  catalog.loaded_reports.markdown.day.labels.date_label = "Date";
  return catalog;
}

}  // namespace

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

  ReportCatalog catalog = BuildMinimalReportCatalog();
  tracer::core::infrastructure::reports::ReportDtoFormatter dto_formatter(
      catalog);
  (void)dto_formatter;

  std::error_code cleanup_error;
  const std::filesystem::path kDbSmokeDir =
      std::filesystem::path("temp") / "phase17_infra_module_reports";
  const std::filesystem::path kDbPath = kDbSmokeDir / "reports.sqlite";
  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  std::filesystem::create_directories(kDbSmokeDir);

  try {
    tracer::core::infrastructure::persistence::importer::sqlite::Connection
        connection(kDbPath.string());
    tracer::core::infrastructure::reports::ReportService report_service(
        connection.GetDb(), catalog, std::make_shared<SmokePlatformClock>());
    static_cast<void>(
        report_service.RunPeriodQuery(7, ReportFormat::kMarkdown));

    auto report_catalog_ptr =
        std::make_shared<ReportCatalog>(BuildMinimalReportCatalog());
    tracer::core::infrastructure::reports::LazySqliteReportQueryService
        lazy_query_service(kDbPath, report_catalog_ptr,
                           std::make_shared<SmokePlatformClock>());
    static_cast<void>(
        lazy_query_service.RunPeriodQuery(7, ReportFormat::kMarkdown));

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

    tracer::core::infrastructure::reports::LazySqliteReportDataQueryService
        lazy_data_query_service(kDbPath,
                                std::make_shared<SmokePlatformClock>());
    if (!lazy_data_query_service.QueryAllDaily().empty()) {
      return 35;
    }
    if (!lazy_data_query_service.ListYearlyTargets().empty()) {
      return 36;
    }
  } catch (...) {
    return 37;
  }

  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  return 0;
}
