// infrastructure/reports/sqlite_report_data_query_service.cpp
#include "infrastructure/reports/sqlite_report_data_query_service.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/reports/data/queriers/daily/daily_querier.hpp"
#include "infrastructure/reports/data/queriers/monthly/monthly_querier.hpp"
#include "infrastructure/reports/data/queriers/period/batch_period_data_fetcher.hpp"
#include "infrastructure/reports/data/queriers/period/period_querier.hpp"
#include "infrastructure/reports/data/queriers/weekly/weekly_querier.hpp"
#include "infrastructure/reports/data/queriers/yearly/yearly_querier.hpp"
#include "infrastructure/reports/services/batch_export_helpers.hpp"

namespace infrastructure::reports {

namespace {

auto EnsureDbConnection(sqlite3* db_connection) -> sqlite3* {
  if (db_connection == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }
  return db_connection;
}

}  // namespace

SqliteReportDataQueryService::SqliteReportDataQueryService(
    sqlite3* db_connection,
    std::shared_ptr<time_tracer::application::ports::IPlatformClock>
        platform_clock)
    : db_connection_(db_connection),
      platform_clock_(std::move(platform_clock)) {
  if (!platform_clock_) {
    throw std::invalid_argument(
        "SqliteReportDataQueryService platform clock must not be null.");
  }
}

auto SqliteReportDataQueryService::QueryDaily(std::string_view date)
    -> DailyReportData {
  DayQuerier querier(EnsureDbConnection(db_connection_), date);
  return querier.FetchData();
}

auto SqliteReportDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyReportData {
  MonthQuerier querier(EnsureDbConnection(db_connection_), month);
  return querier.FetchData();
}

auto SqliteReportDataQueryService::QueryPeriod(int days) -> PeriodReportData {
  PeriodQuerier querier(EnsureDbConnection(db_connection_), days,
                        *platform_clock_);
  return querier.FetchData();
}

auto SqliteReportDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyReportData {
  WeekQuerier querier(EnsureDbConnection(db_connection_), iso_week);
  return querier.FetchData();
}

auto SqliteReportDataQueryService::QueryYearly(std::string_view year)
    -> YearlyReportData {
  YearQuerier querier(EnsureDbConnection(db_connection_), year);
  return querier.FetchData();
}

auto SqliteReportDataQueryService::QueryPeriodBatch(
    const std::vector<int>& days_list) -> std::map<int, PeriodReportData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchPeriodDataFetcher fetcher(db_connection, *platform_clock_);
  auto reports = fetcher.FetchAllData(days_list);

  auto& name_cache = ::reports::services::EnsureProjectNameCache(db_connection);
  for (auto& [days, report] : reports) {
    static_cast<void>(days);
    ::reports::services::EnsureProjectTree(report, name_cache);
  }
  return reports;
}

auto SqliteReportDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyReportData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  auto& name_cache = ::reports::services::EnsureProjectNameCache(db_connection);

  BatchDayDataFetcher fetcher(db_connection, name_cache);
  auto batch_result = fetcher.FetchAllData();

  for (auto& [date, report] : batch_result.data_map) {
    static_cast<void>(date);
    ::reports::services::EnsureProjectTree(report, name_cache);
  }
  return batch_result.data_map;
}

auto SqliteReportDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyReportData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchMonthDataFetcher fetcher(db_connection);
  auto reports = fetcher.FetchAllData();

  auto& name_cache = ::reports::services::EnsureProjectNameCache(db_connection);
  for (auto& [label, report] : reports) {
    static_cast<void>(label);
    ::reports::services::EnsureProjectTree(report, name_cache);
  }
  return reports;
}

auto SqliteReportDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyReportData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchWeekDataFetcher fetcher(db_connection);
  auto reports = fetcher.FetchAllData();

  auto& name_cache = ::reports::services::EnsureProjectNameCache(db_connection);
  for (auto& [label, report] : reports) {
    static_cast<void>(label);
    ::reports::services::EnsureProjectTree(report, name_cache);
  }
  return reports;
}

auto SqliteReportDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyReportData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchYearDataFetcher fetcher(db_connection);
  auto reports = fetcher.FetchAllData();

  auto& name_cache = ::reports::services::EnsureProjectNameCache(db_connection);
  for (auto& [label, report] : reports) {
    static_cast<void>(label);
    ::reports::services::EnsureProjectTree(report, name_cache);
  }
  return reports;
}

}  // namespace infrastructure::reports
