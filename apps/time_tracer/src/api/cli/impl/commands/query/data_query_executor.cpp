// api/cli/impl/commands/query/data_query_executor.cpp
#include "api/cli/impl/commands/query/data_query_executor.hpp"

#include <stdexcept>
#include <utility>

#include "api/cli/impl/commands/query/data_query_output.hpp"
#include "api/cli/impl/commands/query/data_query_parser.hpp"
#include "api/cli/impl/commands/query/data_query_repository.hpp"
#include "api/cli/impl/commands/query/data_query_statistics.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

namespace time_tracer::cli::impl::commands::query::data {
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

}  // namespace
}  // namespace time_tracer::cli::impl::commands::query::data

using namespace time_tracer::cli::impl::commands::query::data;

DataQueryExecutor::DataQueryExecutor(std::filesystem::path db_path)
    : db_path_(std::move(db_path)) {}

void DataQueryExecutor::Execute(const ParsedArgs& args) {
  const auto kFilters = ParseQueryFilters(args);
  const auto kAction = ResolveDataQueryAction(args);

  DBManager db_manager(db_path_.string());
  sqlite3* db_conn = EnsureDbConnectionOrThrow(db_manager, db_path_);

  switch (kAction) {
    case DataQueryAction::kYears:
      PrintList(QueryYears(db_conn));
      return;
    case DataQueryAction::kMonths:
      PrintList(QueryMonths(db_conn, kFilters.year));
      return;
    case DataQueryAction::kDays:
      PrintList(QueryDays(db_conn, kFilters.year, kFilters.month,
                          kFilters.from_date, kFilters.to_date,
                          kFilters.reverse, kFilters.limit));
      return;
    case DataQueryAction::kDaysDuration:
      PrintDayDurations(QueryDayDurations(db_conn, kFilters));
      return;
    case DataQueryAction::kDaysStats: {
      auto stats_filters = kFilters;
      stats_filters.limit.reset();
      stats_filters.reverse = false;
      const auto kRows = QueryDayDurations(db_conn, stats_filters);
      PrintDayDurationStats(ComputeDayDurationStats(kRows));
      if (args.Has("top")) {
        const int kTopN = args.GetAsInt("top");
        PrintTopDayDurations(kRows, kTopN);
      }
      return;
    }
    case DataQueryAction::kSearch:
      PrintList(QueryDatesByFilters(db_conn, kFilters));
      return;
  }

  throw std::runtime_error("Unhandled data query action.");
}
