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
           "Query type (daily, monthly, period)",
           true,
           "",
           0},
          {"argument",
           ArgType::Positional,
           {},
           "Date (YYYYMMDD) or Recent range (days)",
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
  return "Queries statistics (daily, monthly, recent) from the database.";
}

void QueryCommand::execute(const CommandParser& parser) {
  // 1. 统一验证与解析
  ParsedArgs args = CommandValidator::validate(parser, get_definitions());

  // 2. 获取清洗后的参数
  std::string sub_command = args.get("type");
  std::string query_arg = args.get("argument");
  std::string format_str = args.get("format");

  // 3. 业务逻辑
  std::vector<ReportFormat> formats =
      ArgUtils::parse_report_formats(format_str);

  // 预处理日期格式
  if (sub_command == "daily") {
    query_arg = normalize_to_date_format(query_arg);
  } else if (sub_command == "monthly") {
    query_arg = normalize_to_month_format(query_arg);
  }

  // 执行查询
  for (size_t i = 0; i < formats.size(); ++i) {
    ReportFormat format = formats[i];

    if (i > 0) {
      std::cout << "\n" << std::string(40, '=') << "\n";
    }

    if (sub_command == "daily") {
      std::cout << report_handler_.run_daily_query(query_arg, format);
    } else if (sub_command == "monthly") {
      std::cout << report_handler_.run_monthly_query(query_arg, format);
    } else if (sub_command == "period") {
      // [修改] 解析逗号分隔列表，但将循环逻辑下沉到 Core
      std::vector<int> periods;
      std::string token;
      std::istringstream token_stream(query_arg);

      while (std::getline(token_stream, token, ',')) {
        try {
          // 简单的去空格处理
          token.erase(0, token.find_first_not_of(" \t\n\r"));
          token.erase(token.find_last_not_of(" \t\n\r") + 1);

          if (!token.empty()) {
            periods.push_back(std::stoi(token));
          }
        } catch (const std::exception&) {
          std::cerr << "\033[31mError: \033[0mInvalid number '" << token
                    << "' in list. Skipping.\n";
        }
      }

      if (!periods.empty()) {
        // 一次性调用 Core
        std::cout << report_handler_.run_period_queries(periods, format);
      }
    } else {
      throw std::runtime_error("Unknown query type '" + sub_command +
                               "'. Supported: daily, monthly, recent.");
    }
  }
}
