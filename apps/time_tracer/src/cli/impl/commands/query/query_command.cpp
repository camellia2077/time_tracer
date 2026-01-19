#include "query_command.hpp"
#include "cli/framework/core/command_parser.hpp" 
#include "cli/framework/core/command_registry.hpp" 
#include "cli/framework/core/command_validator.hpp" 
#include "cli/impl/utils/arg_utils.hpp"
#include "common/utils/time_utils.hpp" // [修复] 添加此行以解决 normalize_to_* 报错
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>

static CommandRegistrar<AppContext> registrar("query", [](AppContext& ctx) {
    if (!ctx.report_handler) throw std::runtime_error("ReportHandler not initialized");
    return std::make_unique<QueryCommand>(*ctx.report_handler);
});

QueryCommand::QueryCommand(IReportHandler& report_handler)
    : report_handler_(report_handler) {}

// [新增] 定义该命令需要的参数结构
std::vector<ArgDef> QueryCommand::get_definitions() const {
    return {
        // 位置参数 1 (filtered_args[2]): 查询类型
        // 对应: query <type>
        {"type", ArgType::Positional, {}, "Query type (daily, monthly, period)", true, "", 0},
        
        // 位置参数 2 (filtered_args[3]): 查询参数
        // 对应: query <type> <argument>
        {"argument", ArgType::Positional, {}, "Date (YYYYMMDD) or Period (days)", true, "", 1},
        
        // 选项: 输出格式
        // 对应: --format md
        {"format", ArgType::Option, {"-f", "--format"}, "Output format (md, tex, typ)", false, "md"}
    };
}

std::string QueryCommand::get_help() const {
    return "Queries statistics (daily, monthly, period) from the database.";
}

void QueryCommand::execute(const CommandParser& parser) {
    // 1. 统一验证与解析
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());

    // 2. 获取清洗后的参数
    std::string sub_command = args.get("type");
    std::string query_arg = args.get("argument");
    std::string format_str = args.get("format"); 

    // 3. 业务逻辑
    std::vector<ReportFormat> formats = ArgUtils::parse_report_formats(format_str);

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
            // 处理可能的逗号分隔列表 (e.g. query period 7,30)
            std::string token;
            std::istringstream tokenStream(query_arg);
            bool first = true;
            while (std::getline(tokenStream, token, ',')) {
                if (!first) std::cout << "\n" << std::string(40, '-') << "\n";
                try {
                    // 去除可能存在的首尾空格 (简单的健壮性处理)
                    token.erase(0, token.find_first_not_of(" \t\n\r"));
                    token.erase(token.find_last_not_of(" \t\n\r") + 1);
                    
                    std::cout << report_handler_.run_period_query(std::stoi(token), format);
                } catch (const std::exception&) {
                    std::cerr << "\033[31mError: \033[0mInvalid number '" << token << "' in list. Skipping.\n";
                }
                first = false;
            }
        } else {
            throw std::runtime_error("Unknown query type '" + sub_command + "'. Supported: daily, monthly, period.");
        }
    }
}