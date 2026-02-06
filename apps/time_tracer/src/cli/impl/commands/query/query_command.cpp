// cli/impl/commands/query/query_command.cpp
#include "cli/impl/commands/query/query_command.hpp"

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

struct NormalizedQueryArgs {
  std::string_view sub_command;
  std::string_view query_arg;
};

auto NormalizeQueryArgument(const NormalizedQueryArgs& args) -> std::string {
  if (args.sub_command == "day") {
    return NormalizeToDateFormat(std::string(args.query_arg));
  }
  if (args.sub_command == "month") {
    return NormalizeToMonthFormat(std::string(args.query_arg));
  }
  if (args.sub_command == "week") {
    IsoWeek week{};
    if (!ParseIsoWeek(std::string(args.query_arg), week)) {
      throw std::runtime_error(
          "Invalid ISO week format. Use YYYY-Www (e.g., 2026-W05).");
    }
    return FormatIsoWeek(week);
  }
  if (args.sub_command == "year") {
    int gregorian_year = 0;
    if (!ParseGregorianYear(std::string(args.query_arg), gregorian_year)) {
      throw std::runtime_error(
          "Invalid year format. Use Gregorian YYYY (e.g., 2026).");
    }
    return FormatGregorianYear(gregorian_year);
  }
  return std::string(args.query_arg);
}

auto TrimToken(std::string token) -> std::string {
  const size_t kFirst = token.find_first_not_of(kTrimChars);
  if (kFirst == std::string::npos) {
    return std::string{};
  }
  const size_t kLast = token.find_last_not_of(kTrimChars);
  return token.substr(kFirst, kLast - kFirst + 1);
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
                       const NormalizedQueryArgs& args, ReportFormat format) {
  if (args.sub_command == "day") {
    std::cout << report_handler.RunDailyQuery(std::string(args.query_arg),
                                              format);
  } else if (args.sub_command == "month") {
    std::cout << report_handler.RunMonthlyQuery(std::string(args.query_arg),
                                                format);
  } else if (args.sub_command == "recent") {
    std::vector<int> periods = ParseRecentPeriods(std::string(args.query_arg));
    if (!periods.empty()) {
      std::cout << report_handler.RunPeriodQueries(periods, format);
    }
  } else if (args.sub_command == "week") {
    std::cout << report_handler.RunWeeklyQuery(std::string(args.query_arg),
                                               format);
  } else if (args.sub_command == "year") {
    std::cout << report_handler.RunYearlyQuery(std::string(args.query_arg),
                                               format);
  } else {
    throw std::runtime_error("Unknown query type '" +
                             std::string(args.sub_command) +
                             "'. Supported: day, month, week, year, recent.");
  }
}
}  // namespace

static CommandRegistrar<AppContext> registrar(
    "query", [](AppContext& ctx) -> std::unique_ptr<QueryCommand> {
      if (!ctx.report_handler) {
        throw std::runtime_error("ReportHandler not initialized");
      }
      std::string format_value =
          ctx.config.command_defaults.query_format.value_or(
              ctx.config.defaults.default_format.value_or("md"));
      return std::make_unique<QueryCommand>(*ctx.report_handler, format_value);
    });

QueryCommand::QueryCommand(IReportHandler& report_handler,
                           std::string default_format)
    : report_handler_(report_handler),
      default_format_(std::move(default_format)) {}

auto QueryCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"type",
           ArgType::kPositional,
           {},
           "Query type (day, month, week, year, recent)",
           true,
           "",
           0},
          {"argument",
           ArgType::kPositional,
           {},
           "Date (YYYYMMDD), Month (YYYYMM), Week (YYYY-Www), Year (YYYY), or "
           "Recent range (days)",
           true,
           "",
           1},
          {"format",
           ArgType::kOption,
           {"-f", "--format"},
           "Output format (md, tex, typ)",
           false,
           ""}};
}

auto QueryCommand::GetHelp() const -> std::string {
  return "Queries statistics (day, month, week, year, recent) from the "
         "database.";
}

void QueryCommand::Execute(const CommandParser& parser) {
  // 1. 统一验证与解析
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  // 2. 获取清洗后的参数
  const std::string kSubCommand = args.Get("type");
  const std::string kQueryArg = args.Get("argument");
  std::string format_str;
  if (args.Has("format")) {
    format_str = args.Get("format");
  } else {
    format_str = default_format_;
  }

  // 3. 业务逻辑
  const std::vector<ReportFormat> kFormats =
      ArgUtils::ParseReportFormats(format_str);

  const NormalizedQueryArgs kNormalizedArgs = {.sub_command = kSubCommand,
                                               .query_arg = kQueryArg};
  const std::string kNormalizedArgValue =
      NormalizeQueryArgument(kNormalizedArgs);

  // 执行查询
  for (size_t index = 0; index < kFormats.size(); ++index) {
    if (index > 0) {
      std::cout << "\n" << std::string(kSeparatorWidth, '=') << "\n";
    }
    RunQueryForFormat(report_handler_,
                      {.sub_command = kSubCommand,
                       .query_arg = kNormalizedArgValue},
                      kFormats[index]);
  }
}
