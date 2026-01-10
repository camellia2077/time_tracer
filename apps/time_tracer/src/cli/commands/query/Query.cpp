// cli/commands/query/Query.cpp
#include "Query.hpp"
#include "cli/CommandRegistry.hpp" // [新增]
#include <stdexcept>
#include <iostream>
#include <sstream>

// [新增] 静态注册
static CommandRegistrar registrar("query", [](CliContext& ctx) {
    return std::make_unique<Query>(*ctx.report_handler);
});

Query::Query(ReportHandler& report_handler)
    : report_handler_(report_handler) {}

void Query::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 4) {
        throw std::runtime_error("Command 'query' requires a type and a period argument (e.g., query daily 20240101).");
    }
    
    std::string sub_command = filtered_args[2];
    std::string query_arg = filtered_args[3];
    
    // [修改] 获取格式列表
    std::vector<ReportFormat> formats = parser.get_report_formats();

    // [修改] 遍历格式
    for (size_t i = 0; i < formats.size(); ++i) {
        ReportFormat format = formats[i];
        
        // 如果输出多种格式，打印分隔符
        if (i > 0) {
            std::cout << "\n" << std::string(40, '=') << "\n";
        }

        if (sub_command == "daily") {
            std::cout << report_handler_.run_daily_query(query_arg, format);
        } else if (sub_command == "period") {
            std::string token;
            std::istringstream tokenStream(query_arg);
            bool first = true;
            while (std::getline(tokenStream, token, ',')) {
                if (!first) std::cout << "\n" << std::string(40, '-') << "\n";
                try {
                    std::cout << report_handler_.run_period_query(std::stoi(token), format);
                } catch (const std::exception&) {
                    std::cerr << "\033[31mError: \033[0mInvalid number '" << token << "' in list. Skipping.\n";
                }
                first = false;
            }
        } else if (sub_command == "monthly") {
            std::cout << report_handler_.run_monthly_query(query_arg, format);
        } else {
            throw std::runtime_error("Unknown query type '" + sub_command + "'.");
        }
    }
}