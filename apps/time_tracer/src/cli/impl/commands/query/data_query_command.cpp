// cli/impl/commands/query/data_query_command.cpp  
#include "cli/impl/commands/query/data_query_command.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "cli/impl/app/app_context.hpp"
#include "infrastructure/persistence/sqlite/db_manager.hpp"

namespace {
constexpr int kLeapYearDivisor400 = 400;
constexpr int kLeapYearDivisor100 = 100;
constexpr int kLeapYearDivisor4 = 4;

constexpr int kDaysInLongMonth = 31;
constexpr int kDaysInShortMonth = 30;
constexpr int kDaysInLeapFebruary = 29;
constexpr int kDaysInStandardFebruary = 28;

constexpr int kFebruary = 2;
constexpr int kApril = 4;
constexpr int kJune = 6;
constexpr int kSeptember = 9;
constexpr int kNovember = 11;

constexpr int kThresholdTwoDigits = 10;
constexpr int kIsoYearLength = 4;
constexpr int kIsoMonthLength = 6;
constexpr int kIsoDateLength = 8;

constexpr int kYearOffset = 0;
constexpr int kMonthOffset = 4;
constexpr int kDayOffset = 6;

constexpr int kMonthLength = 2;
constexpr int kDayLength = 2;

constexpr int kJanuary = 1;
constexpr int kDecember = 12;
constexpr int kFirstDayOfMonth = 1;
constexpr int kLastDayOfYear = 31;

struct SqlParam {
  enum class Type { kText, kInt };
  Type type = Type::kText;
  std::string text_value;
  int int_value = 0;
};

struct DateParts {
  int year;
  int month;
  int day;
};

struct MonthInfo {
  int year;
  int month;
};

auto EscapeLikeLiteral(const std::string& value) -> std::string {
  std::string escaped;
  escaped.reserve(value.size());
  for (char character : value) {
    if (character == '%' || character == '_' || character == '\\') {
      escaped.push_back('\\');
    }
    escaped.push_back(character);
  }
  return escaped;
}

auto BuildLikeContains(const std::string& value) -> std::string {
  return "%" + EscapeLikeLiteral(value) + "%";
}

auto IsLeapYear(int year) -> bool {
  if (year % kLeapYearDivisor400 == 0) {
    return true;
  }
  if (year % kLeapYearDivisor100 == 0) {
    return false;
  }
  return (year % kLeapYearDivisor4 == 0);
}

auto DaysInMonth(const MonthInfo& info) -> int {
  switch (info.month) {
    case kFebruary:
      return IsLeapYear(info.year) ? kDaysInLeapFebruary : kDaysInStandardFebruary;
    case kApril:
    case kJune:
    case kSeptember:
    case kNovember:
      return kDaysInShortMonth;
    default:
      return kDaysInLongMonth;
  }
}

auto FormatDate(const DateParts& parts) -> std::string {
  std::string month_str = (parts.month < kThresholdTwoDigits)
                              ? ("0" + std::to_string(parts.month))
                              : std::to_string(parts.month);
  std::string day_str = (parts.day < kThresholdTwoDigits)
                            ? ("0" + std::to_string(parts.day))
                            : std::to_string(parts.day);
  return std::to_string(parts.year) + "-" + month_str + "-" + day_str;
}

auto NormalizeDateInput(const std::string& input, bool is_end) -> std::string {
  std::string digits;
  digits.reserve(input.size());
  for (char character : input) {
    if (std::isdigit(static_cast<unsigned char>(character)) != 0) {
      digits.push_back(character);
    }
  }

  if (digits.size() == kIsoYearLength) {
    int year = std::stoi(digits);
    if (is_end) {
      return FormatDate({.year = year, .month = kDecember, .day = kLastDayOfYear});
    }
    return FormatDate(
        {.year = year, .month = kJanuary, .day = kFirstDayOfMonth});
  }
  if (digits.size() == kIsoMonthLength) {
    int year = std::stoi(digits.substr(kYearOffset, kIsoYearLength));
    int month = std::stoi(digits.substr(kMonthOffset, kMonthLength));
    if (is_end) {
      return FormatDate({.year = year,
                         .month = month,
                         .day = DaysInMonth({.year = year, .month = month})});
    }
    return FormatDate({.year = year, .month = month, .day = kFirstDayOfMonth});
  }
  if (digits.size() == kIsoDateLength) {
    int year = std::stoi(digits.substr(kYearOffset, kIsoYearLength));
    int month = std::stoi(digits.substr(kMonthOffset, kMonthLength));
    int day = std::stoi(digits.substr(kDayOffset, kDayLength));
    return FormatDate({.year = year, .month = month, .day = day});
  }

  throw std::runtime_error(
      "Invalid date input. Use YYYY, YYYYMM, or YYYYMMDD.");
}

auto QueryStringColumn(sqlite3* db_conn, const std::string& sql,
                       const std::vector<SqlParam>& params)
    -> std::vector<std::string> {
  std::vector<std::string> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == SqlParam::Type::kInt) {
      sqlite3_bind_int(stmt, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(stmt, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr) {
      results.emplace_back(reinterpret_cast<const char*>(text));
    }
  }
  sqlite3_finalize(stmt);
  return results;
}

auto QueryYearMonth(sqlite3* db_conn, const std::string& sql,
                    const std::vector<SqlParam>& params)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> results;
  sqlite3_stmt* stmt = nullptr;
  if (sqlite3_prepare_v2(db_conn, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    throw std::runtime_error("Failed to prepare query.");
  }

  for (size_t index = 0; index < params.size(); ++index) {
    const auto& param = params[index];
    const int kBindIndex = static_cast<int>(index + 1);
    if (param.type == SqlParam::Type::kInt) {
      sqlite3_bind_int(stmt, kBindIndex, param.int_value);
    } else {
      sqlite3_bind_text(stmt, kBindIndex, param.text_value.c_str(), -1,
                        SQLITE_TRANSIENT);
    }
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    int year = sqlite3_column_int(stmt, 0);
    int month = sqlite3_column_int(stmt, 1);
    results.emplace_back(year, month);
  }
  sqlite3_finalize(stmt);
  return results;
}

auto FormatYearMonth(int year, int month) -> std::string {
  std::string month_str = std::to_string(month);
  if (month < kThresholdTwoDigits) {
    month_str = "0" + month_str;
  }
  return std::to_string(year) + "-" + month_str;
}

auto PrintList(const std::vector<std::string>& items) -> void {
  for (const auto& item : items) {
    std::cout << item << "\n";
  }
  std::cout << "Total: " << items.size() << std::endl;
}

auto QueryYears(sqlite3* db_conn) -> std::vector<std::string> {
  const std::string kSql = "SELECT DISTINCT year FROM days ORDER BY year;";
  return QueryStringColumn(db_conn, kSql, {});
}

auto QueryMonths(sqlite3* db_conn, const std::optional<int>& year)
    -> std::vector<std::string> {
  std::string sql = "SELECT DISTINCT year, month FROM days";
  std::vector<SqlParam> params;
  if (year.has_value()) {
    sql += " WHERE year = ?";
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *year});
  }
  sql += " ORDER BY year, month;";

  std::vector<std::string> formatted;
  for (const auto& [current_year, current_month] :
       QueryYearMonth(db_conn, sql, params)) {
    formatted.push_back(FormatYearMonth(current_year, current_month));
  }
  return formatted;
}

auto QueryDays(sqlite3* db_conn, const std::optional<int>& year,
               const std::optional<int>& month,
               const std::optional<std::string>& from_date,
               const std::optional<std::string>& to_date, bool reverse,
               const std::optional<int>& limit) -> std::vector<std::string> {
  std::string sql = "SELECT date FROM days";
  std::vector<std::string> clauses;
  std::vector<SqlParam> params;

  if (year.has_value()) {
    clauses.emplace_back("year = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *year});
  }
  if (month.has_value()) {
    clauses.emplace_back("month = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *month});
  }
  if (from_date.has_value()) {
    clauses.emplace_back("date >= ?");
    params.push_back({.type = SqlParam::Type::kText, .text_value = *from_date});
  }
  if (to_date.has_value()) {
    clauses.emplace_back("date <= ?");
    params.push_back({.type = SqlParam::Type::kText, .text_value = *to_date});
  }
  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }
  sql += " ORDER BY date ";
  sql += reverse ? "DESC" : "ASC";
  if (limit.has_value() && *limit > 0) {
    sql += " LIMIT ?";
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *limit});
  }
  sql += ";";
  return QueryStringColumn(db_conn, sql, params);
}

auto QueryDatesByFilters(sqlite3* db_conn, const std::optional<int>& year,
                         const std::optional<int>& month,
                         const std::optional<std::string>& day_remark,
                         const std::optional<std::string>& activity_remark,
                         const std::optional<std::string>& project,
                         const std::optional<int>& exercise,
                         const std::optional<int>& status,
                         const std::optional<std::string>& from_date,
                         const std::optional<std::string>& to_date,
                         bool overnight, bool reverse,
                         const std::optional<int>& limit)
    -> std::vector<std::string> {
  std::string sql;
  const bool kNeedRecordsJoin =
      project.has_value() || activity_remark.has_value();

  if (kNeedRecordsJoin && project.has_value()) {
    sql =
        "WITH RECURSIVE project_paths(id, path) AS ("
        "  SELECT id, name FROM projects WHERE parent_id IS NULL"
        "  UNION ALL "
        "  SELECT p.id, pp.path || '_' || p.name "
        "  FROM projects p JOIN project_paths pp ON p.parent_id = pp.id"
        ") "
        "SELECT DISTINCT d.date FROM days d "
        "JOIN time_records tr ON tr.date = d.date "
        "JOIN project_paths pp ON tr.project_id = pp.id";
  } else if (kNeedRecordsJoin) {
    sql =
        "SELECT DISTINCT d.date FROM days d "
        "JOIN time_records tr ON tr.date = d.date";
  } else {
    sql = "SELECT DISTINCT d.date FROM days d";
  }

  std::vector<std::string> clauses;
  std::vector<SqlParam> params;

  if (year.has_value()) {
    clauses.emplace_back("d.year = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *year});
  }
  if (month.has_value()) {
    clauses.emplace_back("d.month = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *month});
  }
  if (from_date.has_value()) {
    clauses.emplace_back("d.date >= ?");
    params.push_back({.type = SqlParam::Type::kText, .text_value = *from_date});
  }
  if (to_date.has_value()) {
    clauses.emplace_back("d.date <= ?");
    params.push_back({.type = SqlParam::Type::kText, .text_value = *to_date});
  }
  if (day_remark.has_value()) {
    clauses.emplace_back("d.remark LIKE ?");
    params.push_back(
        {.type = SqlParam::Type::kText, .text_value = "%" + *day_remark + "%"});
  }
  if (project.has_value()) {
    clauses.emplace_back("pp.path LIKE ? ESCAPE '\\'");
    params.push_back(
        {.type = SqlParam::Type::kText, .text_value = BuildLikeContains(*project)});
  }
  if (activity_remark.has_value()) {
    clauses.emplace_back("tr.activity_remark LIKE ?");
    params.push_back({.type = SqlParam::Type::kText,
                      .text_value = "%" + *activity_remark + "%"});
  }
  if (exercise.has_value()) {
    clauses.emplace_back("d.exercise = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *exercise});
  }
  if (status.has_value()) {
    clauses.emplace_back("d.status = ?");
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *status});
  }
  if (overnight) {
    clauses.emplace_back(
        "(d.getup_time IS NULL OR d.getup_time = '' OR d.getup_time = "
        "'00:00')");
  }

  if (!clauses.empty()) {
    sql += " WHERE " + clauses[0];
    for (size_t i = 1; i < clauses.size(); ++i) {
      sql += " AND " + clauses[i];
    }
  }

  sql += " ORDER BY d.date ";
  sql += reverse ? "DESC" : "ASC";
  if (limit.has_value() && *limit > 0) {
    sql += " LIMIT ?";
    params.push_back({.type = SqlParam::Type::kInt, .int_value = *limit});
  }
  sql += ";";
  return QueryStringColumn(db_conn, sql, params);
}
}  // namespace

static CommandRegistrar<AppContext> registrar(
    "query-data", [](AppContext& ctx) -> std::unique_ptr<DataQueryCommand> {
      return std::make_unique<DataQueryCommand>(ctx.db_path);
    });

DataQueryCommand::DataQueryCommand(std::filesystem::path db_path)
    : db_path_(std::move(db_path)) {}

auto DataQueryCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"years", ArgType::kFlag, {"--years"}, "List distinct years"},
          {"months", ArgType::kFlag, {"--months"}, "List distinct months"},
          {"days", ArgType::kFlag, {"--days"}, "List distinct days"},
          {"year", ArgType::kOption, {"--year"}, "Filter by year"},
          {"month", ArgType::kOption, {"--month"}, "Filter by month"},
          {"from",
           ArgType::kOption,
           {"--from"},
           "Filter start date (YYYY, YYYYMM, YYYYMMDD)"},
          {"to",
           ArgType::kOption,
           {"--to"},
           "Filter end date (YYYY, YYYYMM, YYYYMMDD)"},
          {"remark",
           ArgType::kOption,
           {"--remark"},
           "Filter by activity remark keyword"},
          {"day_remark",
           ArgType::kOption,
           {"--day-remark", "--remark-day"},
           "Filter by day remark keyword"},
          {"project",
           ArgType::kOption,
           {"--project"},
           "Filter by project path (prefix match)"},
          {"overnight",
           ArgType::kFlag,
           {"--overnight"},
           "Filter days with overnight sleep (getup_time is null)"},
          {"exercise",
           ArgType::kOption,
           {"--exercise"},
           "Filter by exercise flag (0 or 1)"},
          {"status",
           ArgType::kOption,
           {"--status"},
           "Filter by status flag (0 or 1)"},
          {"numbers",
           ArgType::kOption,
           {"-n", "--numbers"},
           "Limit number of results"},
          {"reverse",
           ArgType::kFlag,
           {"-r", "--reverse"},
           "Reverse order (date descending)"}};
}

auto DataQueryCommand::GetHelp() const -> std::string {
  return "Queries metadata from the database (list years/months/days or filter "
         "by remark/project/flags).";
}

void DataQueryCommand::Execute(const CommandParser& parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  std::optional<int> year;
  std::optional<int> month;
  if (args.Has("year")) {
    year = args.GetAsInt("year");
  }
  if (args.Has("month")) {
    month = args.GetAsInt("month");
  }
  std::optional<std::string> from_date;
  if (args.Has("from")) {
    from_date = NormalizeDateInput(args.Get("from"), false);
  }
  std::optional<std::string> to_date;
  if (args.Has("to")) {
    to_date = NormalizeDateInput(args.Get("to"), true);
  }

  const bool kListYears = args.Has("years");
  const bool kListMonths = args.Has("months");
  const bool kListDays = args.Has("days");

  std::optional<std::string> remark;
  if (args.Has("remark")) {
    remark = args.Get("remark");
  }
  std::optional<std::string> day_remark;
  if (args.Has("day_remark")) {
    day_remark = args.Get("day_remark");
  }
  std::optional<std::string> project;
  if (args.Has("project")) {
    project = args.Get("project");
  }
  std::optional<int> exercise;
  if (args.Has("exercise")) {
    exercise = args.GetAsInt("exercise");
  }
  std::optional<int> status;
  if (args.Has("status")) {
    status = args.GetAsInt("status");
  }
  const bool kOvernight = args.Has("overnight");
  const bool kReverse = args.Has("reverse");
  std::optional<int> limit;
  if (args.Has("numbers")) {
    limit = args.GetAsInt("numbers");
  }

  DBManager db_manager(db_path_.string());
  if (!db_manager.OpenDatabaseIfNeeded()) {
    throw std::runtime_error("Failed to open database at: " +
                             db_path_.string());
  }
  sqlite3* db_conn = db_manager.GetDbConnection();
  if (db_conn == nullptr) {
    throw std::runtime_error("Database connection is null.");
  }

  if (kListYears) {
    PrintList(QueryYears(db_conn));
    return;
  }

  if (kListMonths) {
    PrintList(QueryMonths(db_conn, year));
    return;
  }

  if (kListDays) {
    PrintList(QueryDays(db_conn, year, month, from_date, to_date, kReverse, limit));
    return;
  }

  const bool kHasFilters = remark.has_value() || day_remark.has_value() ||
                           project.has_value() || exercise.has_value() ||
                           status.has_value() || kOvernight ||
                           year.has_value() || month.has_value() ||
                           from_date.has_value() || to_date.has_value();
  if (!kHasFilters) {
    throw std::runtime_error(
        "No filters provided. Use --years/--months/--days or "
        "add filter options like --from/--to/--remark/--day-remark/--project/"
        "--exercise/--status.");
  }

  PrintList(QueryDatesByFilters(db_conn, year, month, day_remark, remark, project,
                                exercise, status, from_date, to_date, kOvernight,
                                kReverse, limit));
}
