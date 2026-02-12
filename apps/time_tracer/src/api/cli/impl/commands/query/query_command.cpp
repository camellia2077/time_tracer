// api/cli/impl/commands/query/query_command.cpp
#include "api/cli/impl/commands/query/query_command.hpp"

#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/commands/query/data_query_executor.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "api/cli/impl/utils/period_utils.hpp"
#include "domain/utils/time_utils.hpp"
#include "infrastructure/reports/shared/utils/format/iso_week_utils.hpp"
#include "infrastructure/reports/shared/utils/format/year_utils.hpp"

using namespace time_tracer::cli::impl::utils;

namespace {
constexpr std::size_t kSeparatorWidth = 40U;
constexpr std::string_view kSupportedQueryTypes =
    "day, month, week, year, recent, data";

struct NormalizedQueryArgs {
  std::string_view sub_command;
  std::string_view query_arg;
};

using QueryDispatchHandler = void (*)(IReportHandler&, std::string_view,
                                      ReportFormat);

void RunDayQuery(IReportHandler& report_handler, std::string_view query_arg,
                 ReportFormat format) {
  std::cout << report_handler.RunDailyQuery(std::string(query_arg), format);
}

void RunMonthQuery(IReportHandler& report_handler, std::string_view query_arg,
                   ReportFormat format) {
  std::cout << report_handler.RunMonthlyQuery(std::string(query_arg), format);
}

void RunRecentQuery(IReportHandler& report_handler, std::string_view query_arg,
                    ReportFormat format) {
  std::vector<int> periods = ArgUtils::ParseNumberList(std::string(query_arg));
  if (!periods.empty()) {
    std::cout << report_handler.RunPeriodQueries(periods, format);
  }
}

void RunWeekQuery(IReportHandler& report_handler, std::string_view query_arg,
                  ReportFormat format) {
  std::cout << report_handler.RunWeeklyQuery(std::string(query_arg), format);
}

void RunYearQuery(IReportHandler& report_handler, std::string_view query_arg,
                  ReportFormat format) {
  std::cout << report_handler.RunYearlyQuery(std::string(query_arg), format);
}

constexpr std::array<std::pair<std::string_view, QueryDispatchHandler>, 5>
    kQueryDispatchTable = {{
        {"day", &RunDayQuery},
        {"month", &RunMonthQuery},
        {"recent", &RunRecentQuery},
        {"week", &RunWeekQuery},
        {"year", &RunYearQuery},
    }};

[[nodiscard]] auto FindQueryHandler(std::string_view sub_command)
    -> QueryDispatchHandler {
  for (const auto& [name, kHandler] : kQueryDispatchTable) {
    if (name == sub_command) {
      return kHandler;
    }
  }
  return nullptr;
}

void RunQueryForFormat(IReportHandler& report_handler,
                       const NormalizedQueryArgs& args, ReportFormat format) {
  const auto kHandler = FindQueryHandler(args.sub_command);
  if (kHandler == nullptr) {
    throw std::runtime_error(
        "Unknown query type '" + std::string(args.sub_command) +
        "'. Supported: " + std::string(kSupportedQueryTypes) + ".");
  }
  kHandler(report_handler, args.query_arg, format);
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
      // [Update] Pass db_path
      return std::make_unique<QueryCommand>(*ctx.report_handler, format_value,
                                            ctx.db_path.string());
    });

QueryCommand::QueryCommand(IReportHandler& report_handler,
                           std::string default_format, std::string db_path)
    : report_handler_(report_handler),
      default_format_(std::move(default_format)),
      db_path_(std::move(db_path)) {}

auto QueryCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"type",
       ArgType::kPositional,
       {},
       "Query type (day, month, week, year, recent, data)",
       true,
       "",
       0},
      {"argument",
       ArgType::kPositional,
       {},
       "Date/Range arg OR Data Action "
       "(years/months/days/days-duration/days-stats/search)",
       true,
       "",
       1},
      {"format",
       ArgType::kOption,
       {"-f", "--format"},
       "Output format (md, tex, typ)",
       false,
       ""},
      // Data Query Options
      {"year", ArgType::kOption, {"--year"}, "Filter by year", false, ""},
      {"month", ArgType::kOption, {"--month"}, "Filter by month", false, ""},
      {"from",
       ArgType::kOption,
       {"--from"},
       "Filter start date (YYYY, YYYYMM, YYYYMMDD)",
       false,
       ""},
      {"to",
       ArgType::kOption,
       {"--to"},
       "Filter end date (YYYY, YYYYMM, YYYYMMDD)",
       false,
       ""},
      {"remark",
       ArgType::kOption,
       {"--remark"},
       "Filter by activity remark keyword",
       false,
       ""},
      {"day_remark",
       ArgType::kOption,
       {"--day-remark", "--remark-day"},
       "Filter by day remark keyword",
       false,
       ""},
      {"project",
       ArgType::kOption,
       {"--project"},
       "Filter by project path (prefix match)",
       false,
       ""},
      {"overnight",
       ArgType::kFlag,
       {"--overnight"},
       "Filter days with overnight sleep (getup_time is null)",
       false,
       ""},
      {"exercise",
       ArgType::kOption,
       {"--exercise"},
       "Filter by exercise flag (0 or 1)",
       false,
       ""},
      {"status",
       ArgType::kOption,
       {"--status"},
       "Filter by status flag (0 or 1)",
       false,
       ""},
      {"numbers",
       ArgType::kOption,
       {"-n", "--numbers"},
       "Limit number of results",
       false,
       ""},
      {"top",
       ArgType::kOption,
       {"--top"},
       "Top N items for days-stats output",
       false,
       ""},
      {"reverse",
       ArgType::kFlag,
       {"-r", "--reverse"},
       "Reverse order (date descending)",
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
  // 3. 业务逻辑
  if (kSubCommand == "data") {
    DataQueryExecutor executor(db_path_);
    executor.Execute(args);
    return;
  }

  const std::vector<ReportFormat> kFormats =
      ArgUtils::ParseReportFormats(format_str);

  const std::string kNormalizedArgValue = PeriodParser::Normalize(
      {.period_type_ = kSubCommand, .period_arg_ = kQueryArg});

  // 执行查询
  for (size_t index = 0; index < kFormats.size(); ++index) {
    if (index > 0) {
      std::cout << "\n" << std::string(kSeparatorWidth, '=') << "\n";
    }
    RunQueryForFormat(
        report_handler_,
        {.sub_command = kSubCommand, .query_arg = kNormalizedArgValue},
        kFormats[index]);
  }
}
