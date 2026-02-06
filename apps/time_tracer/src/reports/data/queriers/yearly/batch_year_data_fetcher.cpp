// reports/data/queriers/yearly/batch_year_data_fetcher.cpp
#include "reports/data/queriers/yearly/batch_year_data_fetcher.hpp"

#include <map>
#include <set>
#include <stdexcept>

#include "reports/data/cache/project_name_cache.hpp"
#include "reports/data/queriers/utils/batch_aggregation.hpp"
#include "reports/shared/utils/format/year_utils.hpp"

BatchYearDataFetcher::BatchYearDataFetcher(sqlite3* sqlite_db)
    : db_(sqlite_db) {
  if (db_ == nullptr) {
    throw std::invalid_argument("Database connection cannot be null.");
  }
}

auto BatchYearDataFetcher::FetchAllData()
    -> std::map<std::string, YearlyReportData> {

  std::map<std::string, YearlyReportData> results;

  auto& name_cache = ProjectNameCache::Instance();
  name_cache.EnsureLoaded(db_);

  sqlite3_stmt* stmt = nullptr;
  const char* sql =
      "SELECT strftime('%Y', date) as yy, date, project_id, SUM(duration) "
      "FROM time_records "
      "GROUP BY yy, date, project_id "
      "ORDER BY yy;";

  if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare statement for yearly stats.");
  }

  std::map<std::string, std::map<long long, long long>> project_agg;
  std::map<std::string, std::set<std::string>> distinct_dates;

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* yy_ptr = sqlite3_column_text(stmt, 0);
    const unsigned char* date_ptr = sqlite3_column_text(stmt, 1);
    if (yy_ptr == nullptr || date_ptr == nullptr) {
      continue;
    }

    std::string year_str = reinterpret_cast<const char*>(yy_ptr);
    std::string date = reinterpret_cast<const char*>(date_ptr);
    int gregorian_year = 0;
    if (!ParseGregorianYear(year_str, gregorian_year)) {
      continue;
    }

    long long project_id = sqlite3_column_int64(stmt, 2);
    long long duration = sqlite3_column_int64(stmt, 3);

    YearlyReportData& data = results[year_str];
    if (data.range_label.empty()) {
      std::string label = FormatGregorianYear(gregorian_year);
      data.range_label = label;
      data.requested_days = 0;
      data.start_date = label + "-01-01";
      data.end_date = label + "-12-31";
      data.is_valid = true;
    }

    project_agg[year_str][project_id] += duration;
    data.total_duration += duration;
    distinct_dates[year_str].insert(date);
  }
  sqlite3_finalize(stmt);

  reports::data::batch::FinalizeGroupedAggregation(results, project_agg,
                                                   distinct_dates, name_cache);

  return results;
}
