#include <filesystem>
#include <memory>
#include <stdexcept>
#include <utility>

#include "infra/insights/lazy_sqlite_insights_query_service.hpp"
#include "application/compat/insights/i_insights_query_service.hpp"
#include "application/ports/insights/i_platform_clock.hpp"
#include "infra/config/models/insights_catalog.hpp"
#include "infra/persistence/sqlite/db_manager.hpp"

import tracer.core.infrastructure.insights.querying.insights_service;

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
auto WithInsightsService(
    const std::filesystem::path& db_path,
    const std::shared_ptr<InsightsCatalog>& insights_catalog,
    const std::shared_ptr<tracer_core::application::ports::IPlatformClock>&
        platform_clock,
    Callback&& callback) {
  DBManager db_manager(db_path.string());
  InsightsService insights_service(
      EnsureReadableDbConnection(db_path, db_manager), *insights_catalog,
      platform_clock);
  return std::forward<Callback>(callback)(insights_service);
}

}  // namespace

LazySqliteInsightsQueryService::LazySqliteInsightsQueryService(
    std::filesystem::path db_path,
    std::shared_ptr<InsightsCatalog> insights_catalog,
    std::shared_ptr<tracer_core::application::ports::IPlatformClock>
        platform_clock)
    : db_path_(std::move(db_path)),
      insights_catalog_(std::move(insights_catalog)),
      platform_clock_(std::move(platform_clock)) {
  if (db_path_.empty()) {
    throw std::invalid_argument(
        "LazySqliteInsightsQueryService db_path is empty.");
  }
  if (!insights_catalog_) {
    throw std::invalid_argument(
        "LazySqliteInsightsQueryService insights_catalog must not be null.");
  }
  if (!platform_clock_) {
    throw std::invalid_argument(
        "LazySqliteInsightsQueryService platform_clock must not be null.");
  }
}

auto LazySqliteInsightsQueryService::RunDailyQuery(std::string_view date_str,
                                                   InsightsFormat format) const
    -> std::string {
  return WithInsightsService(
      db_path_, insights_catalog_, platform_clock_,
      [&](const InsightsService& insights_service) -> std::string {
        return insights_service.RunDailyQuery(date_str, format);
      });
}

auto LazySqliteInsightsQueryService::RunPeriodQuery(int days,
                                                    InsightsFormat format) const
    -> std::string {
  return WithInsightsService(
      db_path_, insights_catalog_, platform_clock_,
      [&](const InsightsService& insights_service) -> std::string {
        return insights_service.RunPeriodQuery(days, format);
      });
}

auto LazySqliteInsightsQueryService::RunMonthlyQuery(
    std::string_view year_month_str, InsightsFormat format) const
    -> std::string {
  return WithInsightsService(
      db_path_, insights_catalog_, platform_clock_,
      [&](const InsightsService& insights_service) -> std::string {
        return insights_service.RunMonthlyQuery(year_month_str, format);
      });
}

auto LazySqliteInsightsQueryService::RunWeeklyQuery(
    std::string_view iso_week_str, InsightsFormat format) const -> std::string {
  return WithInsightsService(
      db_path_, insights_catalog_, platform_clock_,
      [&](const InsightsService& insights_service) -> std::string {
        return insights_service.RunWeeklyQuery(iso_week_str, format);
      });
}

auto LazySqliteInsightsQueryService::RunYearlyQuery(std::string_view year_str,
                                                    InsightsFormat format) const
    -> std::string {
  return WithInsightsService(
      db_path_, insights_catalog_, platform_clock_,
      [&](const InsightsService& insights_service) -> std::string {
        return insights_service.RunYearlyQuery(year_str, format);
      });
}

}  // namespace tracer::core::infrastructure::insights
