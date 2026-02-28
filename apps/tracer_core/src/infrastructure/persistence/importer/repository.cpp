// infrastructure/persistence/importer/repository.cpp
#include "infrastructure/persistence/importer/repository.hpp"

#include <format>
#include <optional>
#include <stdexcept>

#include "infrastructure/schema/day_schema.hpp"
#include "infrastructure/schema/sqlite_schema.hpp"

namespace infrastructure::persistence::importer {

namespace {

struct MonthBoundary {
  std::string start_date;
  std::string next_month_start_date;
};

[[nodiscard]] auto BuildMonthBoundary(int year, int month)
    -> std::optional<MonthBoundary> {
  constexpr int kMonthsPerYear = 12;
  if (month < 1 || month > kMonthsPerYear) {
    return std::nullopt;
  }

  int next_year = year;
  int next_month = month + 1;
  if (next_month > kMonthsPerYear) {
    next_month = 1;
    ++next_year;
  }

  return MonthBoundary{
      .start_date = std::format("{:04d}-{:02d}-01", year, month),
      .next_month_start_date =
          std::format("{:04d}-{:02d}-01", next_year, next_month)};
}

}  // namespace

Repository::Repository(const std::string& db_path) {
  connection_manager_ = std::make_unique<sqlite::Connection>(db_path);

  if (connection_manager_->GetDb() != nullptr) {
    statement_manager_ =
        std::make_unique<sqlite::Statement>(connection_manager_->GetDb());

    data_inserter_ = std::make_unique<sqlite::Writer>(
        connection_manager_->GetDb(), statement_manager_->GetInsertDayStmt(),
        statement_manager_->GetInsertRecordStmt(),
        statement_manager_->GetInsertProjectStmt());
  }
}

auto Repository::IsDbOpen() const -> bool {
  return connection_manager_ && (connection_manager_->GetDb() != nullptr);
}

auto Repository::ImportData(const std::vector<DayData>& days,
                            const std::vector<TimeRecordInternal>& records)
    -> void {
  if (!IsDbOpen()) {
    throw std::runtime_error("Database is not open. Cannot import data.");
  }

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::ReplaceMonthData(
    const int kYear, const int kMonth, const std::vector<DayData>& days,
    const std::vector<TimeRecordInternal>& records) -> void {
  if (!IsDbOpen()) {
    throw std::runtime_error(
        "Database is not open. Cannot replace month data.");
  }

  const auto kBoundary = BuildMonthBoundary(kYear, kMonth);
  if (!kBoundary.has_value()) {
    throw std::runtime_error("Invalid replace-month target.");
  }

  if (!connection_manager_->BeginTransaction()) {
    throw std::runtime_error("Failed to begin transaction.");
  }

  try {
    const std::string kDeleteRecordsSql = std::format(
        "DELETE FROM {0} WHERE {1} >= '{2}' AND {1} < '{3}';",
        schema::time_records::db::kTable, schema::time_records::db::kDate,
        kBoundary->start_date, kBoundary->next_month_start_date);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), kDeleteRecordsSql,
                            "Delete month rows from time_records")) {
      throw std::runtime_error(
          "Failed to delete month data from time_records.");
    }

    const std::string kDeleteDaysSql =
        std::format("DELETE FROM {0} WHERE {1} >= '{2}' AND {1} < '{3}';",
                    schema::day::db::kTable, schema::day::db::kDate,
                    kBoundary->start_date, kBoundary->next_month_start_date);
    if (!sqlite::ExecuteSql(connection_manager_->GetDb(), kDeleteDaysSql,
                            "Delete month rows from days")) {
      throw std::runtime_error("Failed to delete month data from days.");
    }

    data_inserter_->InsertDays(days);
    data_inserter_->InsertRecords(records);

    if (!connection_manager_->CommitTransaction()) {
      throw std::runtime_error("Failed to commit transaction.");
    }
  } catch (const std::exception&) {
    connection_manager_->RollbackTransaction();
    throw;
  }
}

auto Repository::TryGetLatestActivityTailBeforeDate(std::string_view date) const
    -> std::optional<LatestActivityTail> {
  if (!IsDbOpen()) {
    return std::nullopt;
  }

  const std::string kSql = std::format(
      "SELECT {0}, \"{1}\" FROM {2} WHERE {0} < ?1 "
      "ORDER BY {0} DESC, {3} DESC LIMIT 1;",
      schema::time_records::db::kDate, schema::time_records::db::kEnd,
      schema::time_records::db::kTable,
      schema::time_records::db::kEndTimestamp);

  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(connection_manager_->GetDb(), kSql.c_str(), -1, &stmt,
                         nullptr) != SQLITE_OK) {
    return std::nullopt;
  }

  sqlite3_bind_text(stmt, 1, date.data(), static_cast<int>(date.size()),
                    SQLITE_TRANSIENT);

  std::optional<LatestActivityTail> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* date_text = sqlite3_column_text(stmt, 0);
    const unsigned char* end_text = sqlite3_column_text(stmt, 1);
    if (date_text != nullptr && end_text != nullptr) {
      result = LatestActivityTail{
          .date = reinterpret_cast<const char*>(date_text),
          .end_time = reinterpret_cast<const char*>(end_text)};
    }
  }
  sqlite3_finalize(stmt);
  return result;
}

}  // namespace infrastructure::persistence::importer
