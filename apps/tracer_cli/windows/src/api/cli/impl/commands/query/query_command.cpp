// api/cli/impl/commands/query/query_command.cpp
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/commands/query/data_query_parser.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "api/cli/impl/utils/period_utils.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "shared/types/exceptions.hpp"

class QueryCommand : public ICommand {
public:
  QueryCommand(ITracerCoreApi &core_api, std::string default_format);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "query", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
  std::string default_format_;
};

using namespace tracer_core::cli::impl::utils;
using namespace tracer_core::core::dto;

namespace {
constexpr std::size_t kSeparatorWidth = 40U;
constexpr std::string_view kSupportedQueryTypes =
    "day, month, week, year, recent, range, data";

[[nodiscard]] auto ParseQueryType(std::string_view value)
    -> std::optional<ReportQueryType> {
  if (value == "day") {
    return ReportQueryType::kDay;
  }
  if (value == "month") {
    return ReportQueryType::kMonth;
  }
  if (value == "recent") {
    return ReportQueryType::kRecent;
  }
  if (value == "range") {
    return ReportQueryType::kRange;
  }
  if (value == "week") {
    return ReportQueryType::kWeek;
  }
  if (value == "year") {
    return ReportQueryType::kYear;
  }
  return std::nullopt;
}

auto BuildCoreErrorMessage(std::string_view fallback,
                           const std::string &error_message) -> std::string {
  if (!error_message.empty()) {
    return error_message;
  }
  return std::string(fallback);
}

void EnsureTextOutputSuccess(const TextOutput &response,
                             std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw tracer_core::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterQueryCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "query", [](AppContext &ctx) -> std::unique_ptr<QueryCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        std::string format_value =
            ctx.config.command_defaults.query_format.value_or(
                ctx.config.defaults.default_format.value_or("md"));
        return std::make_unique<QueryCommand>(*ctx.core_api, format_value);
      });
}

} // namespace tracer_core::cli::impl::commands

QueryCommand::QueryCommand(ITracerCoreApi &core_api, std::string default_format)
    : core_api_(core_api), default_format_(std::move(default_format)) {}

auto QueryCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"type",
       ArgType::kPositional,
       {},
       "Query type (day, month, week, year, recent, range, data)",
       true,
       "",
       0},
      {"argument",
       ArgType::kPositional,
       {},
       "Date/Range arg OR Data Action "
       "(years/months/days/days-duration/days-stats/search/activity-suggest/"
       "report-chart/tree)",
       true,
       "",
       1},
      {"format",
       ArgType::kOption,
       {"-f", "--format"},
       "Output format (md, tex, typ)",
       false,
       ""},
      {"data_output",
       ArgType::kOption,
       {"--data-output"},
       "Data query output mode (text, json)",
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
       tracer_core::cli::framework::core::ResolveDeprecatedOptionHelp(
           "query", "--project",
           "Legacy contains filter by project path (prefer --root for root "
           "scope)"),
       false,
       ""},
      {"root",
       ArgType::kOption,
       {"--root"},
       "Recommended root filter (exact root or root_*)",
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
       "Top N items for days-stats/activity-suggest output",
       false,
       ""},
      {"lookback_days",
       ArgType::kOption,
       {"--lookback-days"},
       "Lookback window in days for activity-suggest (default: 10)",
       false,
       ""},
      {"activity_prefix",
       ArgType::kOption,
       {"--activity-prefix"},
       "Activity name prefix filter for activity-suggest",
       false,
       ""},
      {"score_mode",
       ArgType::kOption,
       {"--score-mode"},
       "Score mode for activity-suggest (frequency or duration)",
       false,
       ""},
      {"period",
       ArgType::kOption,
       {"--period"},
       "Period kind for query data tree/days-stats "
       "(day/week/month/year/recent/range)",
       false,
       ""},
      {"period_arg",
       ArgType::kOption,
       {"--period-arg"},
       "Period argument for query data tree/days-stats "
       "(e.g. 20260101, 2026-W05, 7)",
       false,
       ""},
      {"level",
       ArgType::kOption,
       {"-l", "--level"},
       "Max tree depth for query data tree (-1 means unlimited)",
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
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "query", ICommand::GetHelp());
}

void QueryCommand::Execute(const CommandParser &parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kSubCommand = args.Get("type");
  const std::string kQueryArg = args.Get("argument");

  std::string format_str;
  if (args.Has("format")) {
    format_str = args.Get("format");
  } else {
    format_str = default_format_;
  }

  if (parser.HasFlag({"--chart-html-output", "--chart-type", "--chart-theme",
                      "--heatmap-palette", "--list-heatmap-palettes"})) {
    throw std::runtime_error(
        "Chart HTML export options were moved to `chart` command. "
        "Run `chart --help`.");
  }

  if (kSubCommand == "data") {
    DataQueryRequest request =
        tracer_core::cli::impl::commands::query::data::ParseDataQueryRequest(
            args);

    const auto kResponse = core_api_.RunDataQuery(request);
    EnsureTextOutputSuccess(kResponse, "Data query failed.");

    std::cout << kResponse.content;
    return;
  }

  const auto kQueryType = ParseQueryType(kSubCommand);
  if (!kQueryType.has_value()) {
    throw std::runtime_error(
        "Unknown query type '" + kSubCommand +
        "'. Supported: " + std::string(kSupportedQueryTypes) + ".");
  }

  const std::vector<ReportFormat> kFormats =
      ArgUtils::ParseReportFormats(format_str);
  const std::string kNormalizedArgValue = PeriodParser::Normalize(
      {.period_type_ = kSubCommand, .period_arg_ = kQueryArg});

  for (size_t index = 0; index < kFormats.size(); ++index) {
    if (index > 0) {
      std::cout << "\n" << std::string(kSeparatorWidth, '=') << "\n";
    }

    if (*kQueryType == ReportQueryType::kRecent) {
      std::vector<int> periods = ArgUtils::ParseNumberList(kNormalizedArgValue);
      if (periods.empty()) {
        continue;
      }
      const auto kResponse = core_api_.RunPeriodBatchQuery(
          {.days_list = std::move(periods), .format = kFormats[index]});
      EnsureTextOutputSuccess(kResponse, "Recent query failed.");
      std::cout << kResponse.content;
      continue;
    }

    const auto kResponse =
        core_api_.RunReportQuery({.type = *kQueryType,
                                  .argument = kNormalizedArgValue,
                                  .format = kFormats[index]});
    EnsureTextOutputSuccess(kResponse, "Report query failed.");
    std::cout << kResponse.content;
  }
}
