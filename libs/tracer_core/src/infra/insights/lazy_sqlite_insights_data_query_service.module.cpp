#include <filesystem>
#include <memory>
#include <stdexcept>
#include <utility>

#include "infra/insights/lazy_sqlite_insights_data_query_service.hpp"
#include "application/ports/insights/i_platform_clock.hpp"
#include "application/ports/insights/i_insights_data_query_service.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"

import tracer.core.infrastructure.insights.data_querying.sqlite_insights_data_query_service;

namespace tracer::core::infrastructure::insights {
namespace {

auto EnsureReadableDbConnection(const std::filesystem::path& db_path,
                                DBManager& db_manager) -> sqlite3* {
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Insights database is not available: " +
                             db_path.string());
  }

  sqlite3* db_connection = db_manager.GetDbConnection();
  if (db_connection == nullptr) {
    throw std::runtime_error("Insights database connection is null: " +
                             db_path.string());
  }
  return db_connection;
}

template <typename Callback>
auto WithStructuredInsightsService(
    const std::filesystem::path& db_path,
    const std::shared_ptr<tracer_core::application::ports::IPlatformClock>&
        platform_clock,
    Callback&& callback, const DailyStatusConfig* status_config = nullptr) {
  DBManager db_manager(db_path.string());
  SqliteInsightsDataQueryService insights_service(
      EnsureReadableDbConnection(db_path, db_manager), platform_clock,
      status_config != nullptr ? *status_config : DailyStatusConfig{});
  return std::forward<Callback>(callback)(insights_service);
}

}  // namespace

LazySqliteInsightsDataQueryService::LazySqliteInsightsDataQueryService(
    std::filesystem::path db_path,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock,
    std::shared_ptr<const InsightsCatalog> insights_catalog)
    : db_path_(std::move(db_path)),
      platform_clock_(std::move(platform_clock)),
      insights_catalog_(std::move(insights_catalog)) {
  if (db_path_.empty()) {
    throw std::invalid_argument(
        "LazySqliteInsightsDataQueryService db_path is empty.");
  }
  if (!platform_clock_) {
    throw std::invalid_argument(
        "LazySqliteInsightsDataQueryService platform_clock must not be null.");
  }
  if (!insights_catalog_) {
    insights_catalog_ = std::make_shared<InsightsCatalog>();
  }
}

auto LazySqliteInsightsDataQueryService::QueryDaily(std::string_view date)
    -> DailyInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> DailyInsightsData {
        return insights_service.QueryDaily(date);
      },
      &insights_catalog_->statuses.day);
}

auto LazySqliteInsightsDataQueryService::QueryMonthly(std::string_view month)
    -> MonthlyInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> MonthlyInsightsData {
        return insights_service.QueryMonthly(month);
      }, &insights_catalog_->statuses.month);
}

auto LazySqliteInsightsDataQueryService::QueryPeriod(int days)
    -> PeriodInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> PeriodInsightsData {
        return insights_service.QueryPeriod(days);
      }, &insights_catalog_->statuses.recent);
}

auto LazySqliteInsightsDataQueryService::QueryRange(std::string_view start_date,
                                                  std::string_view end_date)
    -> PeriodInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> PeriodInsightsData {
        return insights_service.QueryRange(start_date, end_date);
      }, &insights_catalog_->statuses.range);
}

auto LazySqliteInsightsDataQueryService::QueryWeekly(std::string_view iso_week)
    -> WeeklyInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> WeeklyInsightsData {
        return insights_service.QueryWeekly(iso_week);
      }, &insights_catalog_->statuses.week);
}

auto LazySqliteInsightsDataQueryService::QueryYearly(std::string_view year)
    -> YearlyInsightsData {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service) -> YearlyInsightsData {
        return insights_service.QueryYearly(year);
      }, &insights_catalog_->statuses.year);
}

auto LazySqliteInsightsDataQueryService::ListDailyTargets()
    -> std::vector<std::string> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::vector<std::string> {
        return insights_service.ListDailyTargets();
      });
}

auto LazySqliteInsightsDataQueryService::ListMonthlyTargets()
    -> std::vector<std::string> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::vector<std::string> {
        return insights_service.ListMonthlyTargets();
      });
}

auto LazySqliteInsightsDataQueryService::ListWeeklyTargets()
    -> std::vector<std::string> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::vector<std::string> {
        return insights_service.ListWeeklyTargets();
      });
}

auto LazySqliteInsightsDataQueryService::ListYearlyTargets()
    -> std::vector<std::string> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::vector<std::string> {
        return insights_service.ListYearlyTargets();
      });
}

auto LazySqliteInsightsDataQueryService::QueryPeriodBatch(
    const std::vector<int>& days_list) -> std::map<int, PeriodInsightsData> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::map<int, PeriodInsightsData> {
        return insights_service.QueryPeriodBatch(days_list);
      }, &insights_catalog_->statuses.recent);
}

auto LazySqliteInsightsDataQueryService::QueryAllDaily()
    -> std::map<std::string, DailyInsightsData> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::map<std::string, DailyInsightsData> {
        return insights_service.QueryAllDaily();
      },
      &insights_catalog_->statuses.day);
}

auto LazySqliteInsightsDataQueryService::QueryAllMonthly()
    -> std::map<std::string, MonthlyInsightsData> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::map<std::string, MonthlyInsightsData> {
        return insights_service.QueryAllMonthly();
      }, &insights_catalog_->statuses.month);
}

auto LazySqliteInsightsDataQueryService::QueryAllWeekly()
    -> std::map<std::string, WeeklyInsightsData> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::map<std::string, WeeklyInsightsData> {
        return insights_service.QueryAllWeekly();
      }, &insights_catalog_->statuses.week);
}

auto LazySqliteInsightsDataQueryService::QueryAllYearly()
    -> std::map<std::string, YearlyInsightsData> {
  return WithStructuredInsightsService(
      db_path_, platform_clock_,
      [&](SqliteInsightsDataQueryService& insights_service)
          -> std::map<std::string, YearlyInsightsData> {
        return insights_service.QueryAllYearly();
      }, &insights_catalog_->statuses.year);
}

}  // namespace tracer::core::infrastructure::insights
