module;

#include "infra/sqlite_fwd.hpp"

#include <format>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "application/ports/insights/i_platform_clock.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "infra/insights/data/queriers/daily/daily_querier.hpp"
#include "infra/insights/data/queriers/monthly/monthly_querier.hpp"
#include "infra/insights/data/queriers/period/batch_period_data_fetcher.hpp"
#include "infra/insights/data/queriers/period/period_querier.hpp"
#include "infra/insights/data/queriers/range/date_range_querier.hpp"
#include "infra/insights/data/queriers/weekly/weekly_querier.hpp"
#include "infra/insights/data/queriers/yearly/yearly_querier.hpp"
#include "infra/insights/services/batch_export_helpers.hpp"
#include "infra/schema/sqlite_schema.hpp"
#include "shared/utils/period_utils.hpp"

module tracer.core.infrastructure.insights.data_querying
    .sqlite_insights_data_query_service;

namespace tracer::core::infrastructure::insights {

namespace {

auto EnsureDbConnection(sqlite3* db_connection) -> sqlite3* {
  if (db_connection == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }
  return db_connection;
}

auto LoadDistinctTextTargets(sqlite3* db_connection, const std::string& sql,
                             std::string_view prepare_error)
    -> std::vector<std::string> {
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_connection, sql.c_str(), -1, &stmt, nullptr) !=
      SQLITE_OK) {
    sqlite3_finalize(stmt);
    throw std::runtime_error(std::string(prepare_error));
  }

  std::vector<std::string> items;
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* value_ptr = sqlite3_column_text(stmt, 0);
    if (value_ptr == nullptr) {
      continue;
    }
    items.emplace_back(reinterpret_cast<const char*>(value_ptr));
  }

  sqlite3_finalize(stmt);
  return items;
}

}  // namespace

SqliteInsightsDataQueryService::SqliteInsightsDataQueryService(
    sqlite3* db_connection,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock,
    DailyStatusConfig status_config)
    : db_connection_(db_connection),
      platform_clock_(std::move(platform_clock)),
      status_config_(std::move(status_config)) {
  if (!platform_clock_) {
    throw std::invalid_argument(
        "SqliteInsightsDataQueryService platform clock must not be null.");
  }
}

auto SqliteInsightsDataQueryService::QueryDaily(std::string_view date)
    -> DailyInsightsData {
  DayQuerier querier(EnsureDbConnection(db_connection_), date, &status_config_);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyInsightsData {
  MonthQuerier querier(EnsureDbConnection(db_connection_), month);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::QueryPeriod(int days) -> PeriodInsightsData {
  PeriodQuerier querier(EnsureDbConnection(db_connection_), days,
                        *platform_clock_);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::QueryRange(std::string_view start_date,
                                              std::string_view end_date)
    -> PeriodInsightsData {
  DateRangeQuerier querier(EnsureDbConnection(db_connection_), start_date,
                           end_date);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyInsightsData {
  WeekQuerier querier(EnsureDbConnection(db_connection_), iso_week);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::QueryYearly(std::string_view year)
    -> YearlyInsightsData {
  YearQuerier querier(EnsureDbConnection(db_connection_), year);
  return querier.FetchData();
}

auto SqliteInsightsDataQueryService::ListDailyTargets()
    -> std::vector<std::string> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  const std::string kSql = std::format(
      "SELECT DISTINCT {0} FROM {1} ORDER BY {0};",
      schema::time_records::db::kDate, schema::time_records::db::kTable);
  return LoadDistinctTextTargets(db_connection, kSql,
                                 "Failed to prepare daily target query.");
}

auto SqliteInsightsDataQueryService::ListMonthlyTargets()
    -> std::vector<std::string> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  const std::string kSql = std::format(
      "SELECT DISTINCT strftime('%Y-%m', {0}) FROM {1} ORDER BY 1;",
      schema::time_records::db::kDate, schema::time_records::db::kTable);
  return LoadDistinctTextTargets(db_connection, kSql,
                                 "Failed to prepare monthly target query.");
}

auto SqliteInsightsDataQueryService::ListWeeklyTargets()
    -> std::vector<std::string> {
  const std::vector<std::string> kDates = ListDailyTargets();
  std::vector<std::string> items;
  std::set<std::string> seen;
  items.reserve(kDates.size());
  for (const auto& date : kDates) {
    const IsoWeek week = IsoWeekFromDate(date);
    const std::string label = FormatIsoWeek(week);
    if (seen.insert(label).second) {
      items.push_back(label);
    }
  }
  return items;
}

auto SqliteInsightsDataQueryService::ListYearlyTargets()
    -> std::vector<std::string> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  const std::string kSql = std::format(
      "SELECT DISTINCT strftime('%Y', {0}) FROM {1} ORDER BY 1;",
      schema::time_records::db::kDate, schema::time_records::db::kTable);
  return LoadDistinctTextTargets(db_connection, kSql,
                                 "Failed to prepare yearly target query.");
}

auto SqliteInsightsDataQueryService::QueryPeriodBatch(
    const std::vector<int>& days_list) -> std::map<int, PeriodInsightsData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchPeriodDataFetcher fetcher(db_connection, *platform_clock_);
  auto insights = fetcher.FetchAllData(days_list);

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_connection);
  for (auto& [days, insights] : insights) {
    static_cast<void>(days);
    ::insights::services::EnsureProjectTree(insights, name_cache);
  }
  return insights;
}

auto SqliteInsightsDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyInsightsData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_connection);

  BatchDayDataFetcher fetcher(db_connection, name_cache, &status_config_);
  auto batch_result = fetcher.FetchAllData();

  for (auto& [date, insights] : batch_result.data_map) {
    static_cast<void>(date);
    ::insights::services::EnsureProjectTree(insights, name_cache);
  }
  return batch_result.data_map;
}

auto SqliteInsightsDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyInsightsData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchMonthDataFetcher fetcher(db_connection);
  auto insights = fetcher.FetchAllData();

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_connection);
  for (auto& [label, insights] : insights) {
    static_cast<void>(label);
    ::insights::services::EnsureProjectTree(insights, name_cache);
  }
  return insights;
}

auto SqliteInsightsDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyInsightsData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchWeekDataFetcher fetcher(db_connection);
  auto insights = fetcher.FetchAllData();

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_connection);
  for (auto& [label, insights] : insights) {
    static_cast<void>(label);
    ::insights::services::EnsureProjectTree(insights, name_cache);
  }
  return insights;
}

auto SqliteInsightsDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyInsightsData> {
  sqlite3* db_connection = EnsureDbConnection(db_connection_);
  BatchYearDataFetcher fetcher(db_connection);
  auto insights = fetcher.FetchAllData();

  ProjectNameCache name_cache =
      ::insights::services::CreateProjectNameCache(db_connection);
  for (auto& [label, insights] : insights) {
    static_cast<void>(label);
    ::insights::services::EnsureProjectTree(insights, name_cache);
  }
  return insights;
}

}  // namespace tracer::core::infrastructure::insights
