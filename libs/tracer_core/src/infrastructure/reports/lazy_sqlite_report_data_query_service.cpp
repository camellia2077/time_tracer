#define TT_FORCE_LEGACY_HEADER_DECLS 1
#if TT_ENABLE_CPP20_MODULES
import tracer.core.infrastructure.reports.data_querying.sqlite_report_data_query_service;
#endif

#include "infrastructure/reports/lazy_sqlite_report_data_query_service.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/persistence/sqlite/db_manager.hpp"
#if !TT_ENABLE_CPP20_MODULES
#include "infrastructure/reports/sqlite_report_data_query_service.hpp"
#endif

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
auto WithStructuredReportService(
    const std::filesystem::path& db_path,
    const std::shared_ptr<tracer_core::application::ports::IPlatformClock>&
        platform_clock,
    Callback&& callback) {
  DBManager db_manager(db_path.string());
  SqliteReportDataQueryService report_service(
      EnsureReadableDbConnection(db_path, db_manager), platform_clock);
  return std::forward<Callback>(callback)(report_service);
}

}  // namespace

LazySqliteReportDataQueryService::LazySqliteReportDataQueryService(
    std::filesystem::path db_path,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock)
    : db_path_(std::move(db_path)), platform_clock_(std::move(platform_clock)) {
  if (db_path_.empty()) {
    throw std::invalid_argument(
        "LazySqliteReportDataQueryService db_path is empty.");
  }
  if (!platform_clock_) {
    throw std::invalid_argument(
        "LazySqliteReportDataQueryService platform_clock must not be null.");
  }
}

auto LazySqliteReportDataQueryService::QueryDaily(std::string_view date)
    -> DailyReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryDaily(date);
      });
}

auto LazySqliteReportDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryMonthly(month);
      });
}

auto LazySqliteReportDataQueryService::QueryPeriod(int days)
    -> PeriodReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryPeriod(days);
      });
}

auto LazySqliteReportDataQueryService::QueryRange(std::string_view start_date,
                                                  std::string_view end_date)
    -> PeriodReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryRange(start_date, end_date);
      });
}

auto LazySqliteReportDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryWeekly(iso_week);
      });
}

auto LazySqliteReportDataQueryService::QueryYearly(std::string_view year)
    -> YearlyReportData {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryYearly(year);
      });
}

auto LazySqliteReportDataQueryService::QueryPeriodBatch(
    const std::vector<int>& days_list) -> std::map<int, PeriodReportData> {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryPeriodBatch(days_list);
      });
}

auto LazySqliteReportDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyReportData> {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryAllDaily();
      });
}

auto LazySqliteReportDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyReportData> {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryAllMonthly();
      });
}

auto LazySqliteReportDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyReportData> {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryAllWeekly();
      });
}

auto LazySqliteReportDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyReportData> {
  return WithStructuredReportService(
      db_path_, platform_clock_,
      [&](SqliteReportDataQueryService& report_service) {
        return report_service.QueryAllYearly();
      });
}

}  // namespace tracer::core::infrastructure::reports
