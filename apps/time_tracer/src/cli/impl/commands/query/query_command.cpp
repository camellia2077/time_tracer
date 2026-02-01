// cli/impl/commands/query/query_command.cpp
#include "query_command.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "cli/impl/app/app_context.hpp"
#include "cli/impl/utils/arg_utils.hpp"
#include "domain/utils/time_utils.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"
#include "reports/shared/utils/format/year_utils.hpp"

namespace {
constexpr std::size_t kSeparatorWidth = 40U;
constexpr const char* kTrimChars = " \t\n\r";

auto NormalizeQueryArgument(const std::string& sub_command,
                            const std::string& query_arg) -> std::string {
  if (sub_command == "day") {
    return normalize_to_date_format(query_arg);
  }
  if (sub_command == "month") {
    return normalize_to_month_format(query_arg);
  }
  if (sub_command == "week") {
    IsoWeek week{};
    if (!parse_iso_week(query_arg, week)) {
      throw std::runtime_error(
          "Invalid ISO week format. Use YYYY-Www (e.g., 2026-W05).");
    }
    return format_iso_week(week);
  }
  if (sub_command == "year") {
    int gregorian_year = 0;
    if (!parse_gregorian_year(query_arg, gregorian_year)) {
      throw std::runtime_error(
          "Invalid year format. Use Gregorian YYYY (e.g., 2026).");
    }
    return format_gregorian_year(gregorian_year);
  }
  return query_arg;
}

auto TrimToken(std::string token) -> std::string {
  const size_t first = token.find_first_not_of(kTrimChars);
  if (first == std::string::npos) {
    return std::string{};
  }
  const size_t last = token.find_last_not_of(kTrimChars);
  return token.substr(first, last - first + 1);
}

auto ParseRecentPeriods(const std::string& query_arg) -> std::vector<int> {
  std::vector<int> periods;
  std::string token;
  std::istringstream token_stream(query_arg);

  while (std::getline(token_stream, token, ',')) {
    try {
      std::string trimmed = TrimToken(token);
      if (!trimmed.empty()) {
        periods.push_back(std::stoi(trimmed));
      }
    } catch (const std::exception&) {
      std::cerr << "\033[31mError: \033[0mInvalid number '" << token
                << "' in list. Skipping.\n";
    }
  }

  return periods;
}

void RunQueryForFormat(IReportHandler& report_handler,
                       const std::string& sub_command,
                       const std::string& query_arg, ReportFormat format) {
  if (sub_command == "day") {
    std::cout << report_handler.run_daily_query(query_arg, format);
  } else if (sub_command == "month") {
    std::cout << report_handler.run_monthly_query(query_arg, format);
  } else if (sub_command == "recent") {
    std::vector<int> periods = ParseRecentPeriods(query_arg);
    if (!periods.empty()) {
      std::cout << report_handler.run_period_queries(periods, format);
    }
  } else if (sub_command == "week") {
    std::cout << report_handler.run_weekly_query(query_arg, format);
  } else if (sub_command == "year") {
    std::cout << report_handler.run_yearly_query(query_arg, format);
  } else {
    throw std::runtime_error("Unknown query type '" + sub_command +
                             "'. Supported: day, month, week, year, recent.");
  }
}
}  // namespace

static CommandRegistrar<AppContext> registrar(
    "query", [](AppContext& ctx) -> std::unique_ptr<QueryCommand> {
      if (!ctx.report_handler) {
        throw std::runtime_error("ReportHandler not initialized");
      }
      return std::make_unique<QueryCommand>(*ctx.report_handler);
    });

QueryCommand::QueryCommand(IReportHandler& report_handler)
    : report_handler_(report_handler) {}

auto QueryCommand::get_definitions() const -> std::vector<ArgDef> {
  return {{"type",
           ArgType::Positional,
           {},
           "Query type (day, month, week, year, recent)",
           true,
           "",
           0},
          {"argument",
           ArgType::Positional,
           {},
           "Date (YYYYMMDD), Month (YYYYMM), Week (YYYY-Www), Year (YYYY), or "
           "Recent range (days)",
           true,
           "",
           1},
          {"format",
           ArgType::Option,
           {"-f", "--format"},
           "Output format (md, tex, typ)",
           false,
           "md"}};
}

auto QueryCommand::get_help() const -> std::string {
  return "Queries statistics (day, month, week, year, recent) from the "
         "database.";
}

void QueryCommand::execute(const CommandParser& parser) {
  // 1. 统一验证与解析
  ParsedArgs args = CommandValidator::validate(parser, get_definitions());

  // 2. 获取清洗后的参数
  const std::string sub_command = args.get("type");
  const std::string query_arg = args.get("argument");
  const std::string format_str = args.get("format");

  // 3. 业务逻辑
  const std::vector<ReportFormat> formats =
      ArgUtils::parse_report_formats(format_str);

  const std::string normalized_arg =
      NormalizeQueryArgument(sub_command, query_arg);

  // 执行查询
  for (size_t index = 0; index < formats.size(); ++index) {
    if (index > 0) {
      std::cout << "\n" << std::string(kSeparatorWidth, '=') << "\n";
    }
    RunQueryForFormat(report_handler_, sub_command, normalized_arg,
                      formats[index]);
  }
}
