#include "export_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp" // [新增]
#include "cli/impl/utils/arg_utils.hpp"
#include "common/utils/time_utils.hpp"
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>

static CommandRegistrar<AppContext> registrar("export", [](AppContext& ctx) {
    if (!ctx.report_handler) throw std::runtime_error("ReportHandler not initialized");
    return std::make_unique<ExportCommand>(*ctx.report_handler);
});

ExportCommand::ExportCommand(IReportHandler& report_handler)
    : report_handler_(report_handler) {}

std::vector<ArgDef> ExportCommand::get_definitions() const {
    return {
        {"type", ArgType::Positional, {}, "Export type (daily, monthly, period, all-daily...)", true, "", 0},
        // argument 设为非必填，因为 all-daily 等不需要参数
        {"argument", ArgType::Positional, {}, "Date or Period", false, "", 1},
        {"format", ArgType::Option, {"-f", "--format"}, "Output format", false, "md"},
        {"output", ArgType::Option, {"-o", "--output"}, "Output directory", false, ""},
        {"db", ArgType::Option, {"--db", "--database"}, "Database path", false, ""}
    };
}

std::string ExportCommand::get_help() const {
    return "Exports reports (daily, monthly, period, etc.) to specified formats (md, tex, typ).";
}

void ExportCommand::execute(const CommandParser& parser) {
    // 1. 统一验证
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());

    std::string sub_command = args.get("type");
    std::string export_arg = args.get("argument"); // 可能为空
    
    std::vector<ReportFormat> formats;
    if (args.has("format")) {
        formats = ArgUtils::parse_report_formats(args.get("format"));
    } else {
        formats = {ReportFormat::Markdown};
    }

    // 2. 遍历所有请求的格式进行导出
    for (const auto& format : formats) {
        if (sub_command == "daily" || sub_command == "monthly" || sub_command == "period" || sub_command == "all-period") {
            // 这些命令必须要有 argument
            if (export_arg.empty()) {
                throw std::runtime_error("Argument required for export type '" + sub_command + "'.");
            }

            if (sub_command == "daily") {
                std::string date_str = normalize_to_date_format(export_arg);
                report_handler_.run_export_single_day_report(date_str, format);
            } else if (sub_command == "monthly") {
                std::string month_str = normalize_to_month_format(export_arg);
                report_handler_.run_export_single_month_report(month_str, format);
            } else if (sub_command == "period") {
                try {
                    report_handler_.run_export_single_period_report(std::stoi(export_arg), format);
                } catch (const std::exception&) {
                    throw std::runtime_error("Invalid number provided for 'export period': " + export_arg);
                }
            } else if (sub_command == "all-period") {
                std::vector<int> days_list;
                std::string token;
                std::istringstream tokenStream(export_arg);
                while (std::getline(tokenStream, token, ',')) {
                    try {
                        days_list.push_back(std::stoi(token));
                    } catch (const std::exception&) {
                        throw std::runtime_error("Invalid number in days list for 'export all-period': " + token);
                    }
                }
                report_handler_.run_export_all_period_reports_query(days_list, format);
            }
        }
        else if (sub_command == "all-daily") {
            report_handler_.run_export_all_daily_reports_query(format);
        } else if (sub_command == "all-monthly") {
            report_handler_.run_export_all_monthly_reports_query(format);
        } else {
            throw std::runtime_error("Unknown export type '" + sub_command + "'.");
        }
    }
}