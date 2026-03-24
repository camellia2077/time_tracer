#include <filesystem>
#include <memory>
#include <stdexcept>
#include <utility>

#include "infra/reporting/lazy_sqlite_report_query_service.hpp"
#include "application/compat/reporting/i_report_query_service.hpp"
#include "application/ports/reporting/i_platform_clock.hpp"
#include "infra/config/models/report_catalog.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"

import tracer.core.infrastructure.reporting.querying.report_service;

namespace tracer::core::infrastructure::reports {
namespace {

auto EnsureReadableDbConnection(const std::filesystem::path& db_path,
                                DBManager& db_manager) -> sqlite3* {
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Report database is not available: " +
                             db_path.string());
  }

  sqlite3* db_connection = db_manager.GetDbConnection();
  if (db_connection == nullptr) {
    throw std::runtime_error("Report database connection is null: " +
                             db_path.string());
  }
  return db_connection;
}

template <typename Callback>
auto WithReportService(
    const std::filesystem::path& db_path,
    const std::shared_ptr<ReportCatalog>& report_catalog,
    const std::shared_ptr<tracer_core::application::ports::IPlatformClock>&
        platform_clock,
    Callback&& callback) {
  DBManager db_manager(db_path.string());
  ReportService report_service(
      EnsureReadableDbConnection(db_path, db_manager), *report_catalog,
      platform_clock);
  return std::forward<Callback>(callback)(report_service);
}

}  // namespace

LazySqliteReportQueryService::LazySqliteReportQueryService(
    std::filesystem::path db_path, std::shared_ptr<ReportCatalog> report_catalog,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock)
    : db_path_(std::move(db_path)),
      report_catalog_(std::move(report_catalog)),
      platform_clock_(std::move(platform_clock)) {
  if (db_path_.empty()) {
    throw std::invalid_argument("LazySqliteReportQueryService db_path is empty.");
  }
  if (!report_catalog_) {
    throw std::invalid_argument(
        "LazySqliteReportQueryService report_catalog must not be null.");
  }
  if (!platform_clock_) {
    throw std::invalid_argument(
        "LazySqliteReportQueryService platform_clock must not be null.");
  }
}

auto LazySqliteReportQueryService::RunDailyQuery(std::string_view date_str,
                                                 ReportFormat format) const
    -> std::string {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> std::string {
                             return report_service.RunDailyQuery(date_str,
                                                                 format);
                           });
}

auto LazySqliteReportQueryService::RunPeriodQuery(int days,
                                                  ReportFormat format) const
    -> std::string {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> std::string {
                             return report_service.RunPeriodQuery(days, format);
                           });
}

auto LazySqliteReportQueryService::RunMonthlyQuery(
    std::string_view year_month_str, ReportFormat format) const -> std::string {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> std::string {
                             return report_service.RunMonthlyQuery(
                                 year_month_str, format);
                           });
}

auto LazySqliteReportQueryService::RunWeeklyQuery(
    std::string_view iso_week_str, ReportFormat format) const -> std::string {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> std::string {
                             return report_service.RunWeeklyQuery(iso_week_str,
                                                                  format);
                           });
}

auto LazySqliteReportQueryService::RunYearlyQuery(
    std::string_view year_str, ReportFormat format) const -> std::string {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> std::string {
                             return report_service.RunYearlyQuery(year_str,
                                                                  format);
                           });
}

auto LazySqliteReportQueryService::RunExportAllDailyReportsQuery(
    ReportFormat format) const -> FormattedGroupedReports {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> FormattedGroupedReports {
                             return report_service.RunExportAllDailyReportsQuery(
                                 format);
                           });
}

auto LazySqliteReportQueryService::RunExportAllMonthlyReportsQuery(
    ReportFormat format) const -> FormattedMonthlyReports {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> FormattedMonthlyReports {
                             return report_service
                                 .RunExportAllMonthlyReportsQuery(format);
                           });
}

auto LazySqliteReportQueryService::RunExportAllPeriodReportsQuery(
    const std::vector<int>& days_list, ReportFormat format) const
    -> FormattedPeriodReports {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> FormattedPeriodReports {
                             return report_service.RunExportAllPeriodReportsQuery(
                                 days_list, format);
                           });
}

auto LazySqliteReportQueryService::RunExportAllWeeklyReportsQuery(
    ReportFormat format) const -> FormattedWeeklyReports {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> FormattedWeeklyReports {
                             return report_service
                                 .RunExportAllWeeklyReportsQuery(format);
                           });
}

auto LazySqliteReportQueryService::RunExportAllYearlyReportsQuery(
    ReportFormat format) const -> FormattedYearlyReports {
  return WithReportService(db_path_, report_catalog_, platform_clock_,
                           [&](const ReportService& report_service)
                               -> FormattedYearlyReports {
                             return report_service
                                 .RunExportAllYearlyReportsQuery(format);
                           });
}

}  // namespace tracer::core::infrastructure::reports
