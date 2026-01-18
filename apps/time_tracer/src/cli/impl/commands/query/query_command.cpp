// cli/impl/commands/query/query_command.cpp
#include "query_command.hpp"
#include "cli/framework/core/command_registry.hpp" 
#include "cli/impl/utils/arg_utils.hpp"
#include "common/utils/time_utils.hpp"
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>

static CommandRegistrar<AppContext> registrar("query", [](AppContext& ctx) {
    return std::make_unique<QueryCommand>(*ctx.report_handler);
});

QueryCommand::QueryCommand(ReportHandler& report_handler)
    : report_handler_(report_handler) {}

// [Fixed] CommandParser
void QueryCommand::execute(const CommandParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 4) {
        throw std::runtime_error("Command 'query' requires a type and a period argument (e.g., query daily 20240101).");
    }
    
    std::string sub_command = filtered_args[2];
    std::string query_arg = filtered_args[3];
    
    std::vector<ReportFormat> formats;
    auto format_str = parser.get_option({"-f", "--format"});
    
    if (format_str) {
        formats = ArgUtils::parse_report_formats(*format_str);
    } else {
        formats = {ReportFormat::Markdown}; 
    }

    if (sub_command == "daily") {
        query_arg = normalize_to_date_format(query_arg);
    } else if (sub_command == "monthly") {
        query_arg = normalize_to_month_format(query_arg);
    }

    for (size_t i = 0; i < formats.size(); ++i) {
        ReportFormat format = formats[i];
        
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