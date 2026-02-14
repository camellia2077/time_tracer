// infrastructure/persistence/sqlite_data_query_service.cpp
#include "infrastructure/persistence/sqlite_data_query_service.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/persistence/sqlite/db_manager.hpp"
#include "infrastructure/query/data/data_query_models.hpp"
#include "infrastructure/query/data/data_query_repository.hpp"

namespace infra_data_query = time_tracer::infrastructure::query::data;

namespace {

auto EnsureDbConnectionOrThrow(DBManager& db_manager,
                               const std::filesystem::path& db_path)
    -> sqlite3* {
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database at: " + db_path.string());
  }

  sqlite3* db_conn = db_manager.GetDbConnection();
  if (db_conn == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }
  return db_conn;
}

auto ToCliDataQueryAction(time_tracer::core::dto::DataQueryAction action)
    -> infra_data_query::DataQueryAction {
  using CoreAction = time_tracer::core::dto::DataQueryAction;
  switch (action) {
    case CoreAction::kYears:
      return infra_data_query::DataQueryAction::kYears;
    case CoreAction::kMonths:
      return infra_data_query::DataQueryAction::kMonths;
    case CoreAction::kDays:
      return infra_data_query::DataQueryAction::kDays;
    case CoreAction::kDaysDuration:
      return infra_data_query::DataQueryAction::kDaysDuration;
    case CoreAction::kDaysStats:
      return infra_data_query::DataQueryAction::kDaysStats;
    case CoreAction::kSearch:
      return infra_data_query::DataQueryAction::kSearch;
  }
  throw std::runtime_error("Unsupported data query action.");
}

auto BuildCliFilters(const time_tracer::core::dto::DataQueryRequest& request)
    -> infra_data_query::QueryFilters {
  infra_data_query::QueryFilters filters;
  filters.kYear = request.year;
  filters.kMonth = request.month;
  filters.from_date = request.from_date;
  filters.to_date = request.to_date;
  filters.remark = request.remark;
  filters.day_remark = request.day_remark;
  filters.project = request.project;
  filters.exercise = request.exercise;
  filters.status = request.status;
  filters.overnight = request.overnight;
  filters.reverse = request.reverse;
  filters.limit = request.limit;
  return filters;
}

}  // namespace

namespace infrastructure::persistence {

SqliteDataQueryService::SqliteDataQueryService(std::filesystem::path db_path)
    : db_path_(std::move(db_path)) {}

auto SqliteDataQueryService::RunDataQuery(
    const time_tracer::core::dto::DataQueryRequest& request)
    -> time_tracer::core::dto::TextOutput {
  DBManager db_manager(db_path_.string());
  sqlite3* db_conn = EnsureDbConnectionOrThrow(db_manager, db_path_);

  const auto kAction = ToCliDataQueryAction(request.action);
  const auto kFilters = BuildCliFilters(request);

  switch (kAction) {
    case infra_data_query::DataQueryAction::kYears:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryYears(db_conn)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kMonths:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryMonths(db_conn, kFilters.kYear)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kDays:
      return {
          .ok = true,
          .content = infra_data_query::RenderList(infra_data_query::QueryDays(
              db_conn, kFilters.kYear, kFilters.kMonth, kFilters.from_date,
              kFilters.to_date, kFilters.reverse, kFilters.limit)),
          .error_message = ""};
    case infra_data_query::DataQueryAction::kDaysDuration:
      return {.ok = true,
              .content = infra_data_query::RenderDayDurations(
                  infra_data_query::QueryDayDurations(db_conn, kFilters)),
              .error_message = ""};
    case infra_data_query::DataQueryAction::kDaysStats: {
      auto stats_filters = kFilters;
      stats_filters.limit.reset();
      stats_filters.reverse = false;
      const auto kRows =
          infra_data_query::QueryDayDurations(db_conn, stats_filters);
      std::string content = infra_data_query::RenderDayDurationStats(
          infra_data_query::ComputeDayDurationStats(kRows));
      if (request.top_n.has_value()) {
        content +=
            infra_data_query::RenderTopDayDurations(kRows, *request.top_n);
      }
      return {.ok = true, .content = std::move(content), .error_message = ""};
    }
    case infra_data_query::DataQueryAction::kSearch:
      return {.ok = true,
              .content = infra_data_query::RenderList(
                  infra_data_query::QueryDatesByFilters(db_conn, kFilters)),
              .error_message = ""};
  }

  throw std::runtime_error("Unhandled data query action.");
}

}  // namespace infrastructure::persistence
