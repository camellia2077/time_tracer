// cli/commands/export/Export.cpp
#include "Export.hpp"
#include "cli/CommandRegistry.hpp"
#include "common/utils/TimeUtils.hpp" // [新增]
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>

static CommandRegistrar registrar("export", [](CliContext& ctx) {
    return std::make_unique<Export>(*ctx.report_handler);
});

Export::Export(ReportHandler& report_handler)
    : report_handler_(report_handler) {}

void Export::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'export' requires a type argument (e.g., export daily).");
    }

    std::string sub_command = filtered_args[2];
    std::vector<ReportFormat> formats = parser.get_report_formats();

    for (const auto& format : formats) {
        if (sub_command == "daily" || sub_command == "monthly" || sub_command == "period" || sub_command == "all-period") {
            if (filtered_args.size() < 4) {
                throw std::runtime_error("Argument required for export type '" + sub_command + "'.");
            }
            std::string export_arg = filtered_args[3];

            // --- [新增] 自动格式化日期输入 ---
            if (sub_command == "daily") {
                export_arg = normalize_to_date_format(export_arg);
            } else if (sub_command == "monthly") {
                export_arg = normalize_to_month_format(export_arg);
            }
            // -------------------------------

            if (sub_command == "daily") {
                report_handler_.run_export_single_day_report(export_arg, format);
            } else if (sub_command == "monthly") {
                report_handler_.run_export_single_month_report(export_arg, format);
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