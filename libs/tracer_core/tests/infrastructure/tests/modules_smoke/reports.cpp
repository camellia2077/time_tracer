import tracer.core.infrastructure.persistence.write;
import tracer.core.infrastructure.reports.data_querying;
import tracer.core.infrastructure.reports.dto;
import tracer.core.infrastructure.reports.exporting;
import tracer.core.infrastructure.reports.querying;

#include "application/ports/i_report_dto_formatter.hpp"
#include "infrastructure/tests/modules_smoke/support.hpp"

namespace {

auto BuildMinimalReportCatalog() -> ReportCatalog {
  ReportCatalog catalog;
  catalog.loaded_reports.markdown.day.labels.date_label = "Date";
  return catalog;
}

class StubReportDtoFormatter final
    : public tracer_core::application::ports::IReportDtoFormatter {
 public:
  auto FormatDaily(const DailyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "daily:" + report.date;
  }

  auto FormatMonthly(const MonthlyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "monthly:" + report.range_label;
  }

  auto FormatPeriod(const PeriodReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "period:" + std::to_string(report.requested_days);
  }

  auto FormatWeekly(const WeeklyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "weekly:" + report.range_label;
  }

  auto FormatYearly(const YearlyReportData& report, ReportFormat /*format*/)
      -> std::string override {
    return "yearly:" + report.range_label;
  }
};

class RecordingExporter final : public IReportExporter {
 public:
  mutable std::string last_id;
  mutable std::string last_content;
  mutable FormattedYearlyReports last_yearly_reports;

  auto ExportSingleDayReport(const SingleExportTask& task,
                             ReportFormat /*format*/) const -> void override {
    last_id = std::string(task.id);
    last_content = std::string(task.kContent);
  }

  auto ExportSingleMonthReport(const SingleExportTask& task,
                               ReportFormat /*format*/) const
      -> void override {
    last_id = std::string(task.id);
    last_content = std::string(task.kContent);
  }

  auto ExportSinglePeriodReport(int /*days*/, std::string_view content,
                                ReportFormat /*format*/) const
      -> void override {
    last_content = std::string(content);
  }

  auto ExportSingleWeekReport(const SingleExportTask& task,
                              ReportFormat /*format*/) const -> void override {
    last_id = std::string(task.id);
    last_content = std::string(task.kContent);
  }

  auto ExportSingleYearReport(const SingleExportTask& task,
                              ReportFormat /*format*/) const -> void override {
    last_id = std::string(task.id);
    last_content = std::string(task.kContent);
  }

  auto ExportAllDailyReports(const FormattedGroupedReports& /*reports*/,
                             ReportFormat /*format*/) const -> void override {}

  auto ExportAllMonthlyReports(const FormattedMonthlyReports& /*reports*/,
                               ReportFormat /*format*/) const
      -> void override {}

  auto ExportAllPeriodReports(const FormattedPeriodReports& /*reports*/,
                              ReportFormat /*format*/) const -> void override {}

  auto ExportAllWeeklyReports(const FormattedWeeklyReports& /*reports*/,
                              ReportFormat /*format*/) const -> void override {}

  auto ExportAllYearlyReports(const FormattedYearlyReports& reports,
                              ReportFormat /*format*/) const -> void override {
    last_yearly_reports = reports;
  }
};

}  // namespace

auto RunInfrastructureModuleReportsSmoke() -> int {
  const auto kGetReportFormatDetails =
      &tracer::core::infrastructure::reports::GetReportFormatDetails;
  const auto kExecuteExportTask =
      &tracer::core::infrastructure::reports::ExecuteExportTask;
  const auto kFormatDaily =
      &tracer::core::infrastructure::reports::ReportDtoFormatter::FormatDaily;
  const auto kExportSingleDay =
      &tracer::core::infrastructure::reports::ReportDtoExportWriter::
          ExportSingleDay;
  const auto kRunExportAllPeriodReportsQuery =
      &tracer::core::infrastructure::reports::ReportService::
          RunExportAllPeriodReportsQuery;
  const auto kGenerateAllDailyReports =
      &tracer::core::infrastructure::reports::services::DailyReportService::
          GenerateAllReports;
  const auto kGenerateMonthlyReports =
      &tracer::core::infrastructure::reports::services::MonthlyReportService::
          GenerateReports;
  const auto kGenerateWeeklyReports =
      &tracer::core::infrastructure::reports::services::WeeklyReportService::
          GenerateReports;
  const auto kGenerateYearlyReports =
      &tracer::core::infrastructure::reports::services::YearlyReportService::
          GenerateReports;
  const auto kQueryAllDaily =
      &tracer::core::infrastructure::reports::SqliteReportDataQueryService::
          QueryAllDaily;
  const auto kQueryAllYearly =
      &tracer::core::infrastructure::reports::LazySqliteReportDataQueryService::
          QueryAllYearly;
  (void)kGetReportFormatDetails;
  (void)kExecuteExportTask;
  (void)kFormatDaily;
  (void)kExportSingleDay;
  (void)kRunExportAllPeriodReportsQuery;
  (void)kGenerateAllDailyReports;
  (void)kGenerateMonthlyReports;
  (void)kGenerateWeeklyReports;
  (void)kGenerateYearlyReports;
  (void)kQueryAllDaily;
  (void)kQueryAllYearly;

  const auto kMarkdownDetails =
      tracer::core::infrastructure::reports::GetReportFormatDetails(
          ReportFormat::kMarkdown);
  if (!kMarkdownDetails.has_value() ||
      kMarkdownDetails->dir_name != "markdown" ||
      kMarkdownDetails->extension != ".md") {
    return 31;
  }

  std::error_code cleanup_error;
  const std::filesystem::path kExportSmokeDir =
      std::filesystem::path("temp") / "phase16_infra_module_reports";
  std::filesystem::remove_all(kExportSmokeDir, cleanup_error);

  tracer::core::infrastructure::reports::ReportFileManager file_manager(
      kExportSmokeDir);
  if (file_manager
          .GetSingleWeekReportPath("2026-W09", ReportFormat::kMarkdown)
          .extension() != ".md") {
    return 32;
  }

  tracer::core::infrastructure::reports::Exporter exporter(kExportSmokeDir);
  exporter.ExportSingleDayReport(
      {.id = "2026-02-03", .kContent = "module smoke export\n"},
      ReportFormat::kMarkdown);
  if (!std::filesystem::exists(kExportSmokeDir / "markdown" / "day" / "2026" /
                               "02" / "2026-02-03.md")) {
    return 33;
  }

  ReportCatalog catalog = BuildMinimalReportCatalog();
  tracer::core::infrastructure::reports::ReportDtoFormatter dto_formatter(
      catalog);
  (void)dto_formatter;

  auto recording_exporter = std::make_shared<RecordingExporter>();
  auto stub_formatter = std::make_shared<StubReportDtoFormatter>();
  tracer::core::infrastructure::reports::ReportDtoExportWriter export_writer(
      stub_formatter, recording_exporter);

  DailyReportData daily_report;
  daily_report.date = "2026-02-03";
  export_writer.ExportSingleDay("2026-02-03", daily_report,
                                ReportFormat::kMarkdown);
  if (recording_exporter->last_id != "2026-02-03" ||
      recording_exporter->last_content != "daily:2026-02-03") {
    return 34;
  }

  YearlyReportData yearly_report;
  yearly_report.range_label = "2026";
  export_writer.ExportAllYearly({{"2026", yearly_report}},
                                ReportFormat::kMarkdown);
  if (recording_exporter->last_yearly_reports.size() != 1U ||
      !recording_exporter->last_yearly_reports.contains(2026)) {
    return 35;
  }

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
    if (!report_service
             .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
             .empty()) {
      return 36;
    }

    auto report_catalog_ptr =
        std::make_shared<ReportCatalog>(BuildMinimalReportCatalog());
    tracer::core::infrastructure::reports::LazySqliteReportQueryService
        lazy_query_service(kDbPath, report_catalog_ptr,
                           std::make_shared<SmokePlatformClock>());
    if (!lazy_query_service
             .RunExportAllPeriodReportsQuery({}, ReportFormat::kMarkdown)
             .empty()) {
      return 37;
    }

    tracer::core::infrastructure::reports::SqliteReportDataQueryService
        data_query_service(connection.GetDb(),
                           std::make_shared<SmokePlatformClock>());
    if (!data_query_service.QueryAllDaily().empty()) {
      return 38;
    }

    tracer::core::infrastructure::reports::LazySqliteReportDataQueryService
        lazy_data_query_service(kDbPath, std::make_shared<SmokePlatformClock>());
    if (!lazy_data_query_service.QueryAllDaily().empty()) {
      return 39;
    }
    if (!data_query_service.QueryPeriodBatch({}).empty()) {
      return 40;
    }
    if (!lazy_data_query_service.QueryAllYearly().empty()) {
      return 41;
    }
  } catch (...) {
    return 36;
  }

  std::filesystem::remove_all(kExportSmokeDir, cleanup_error);
  std::filesystem::remove_all(kDbSmokeDir, cleanup_error);
  return 0;
}
