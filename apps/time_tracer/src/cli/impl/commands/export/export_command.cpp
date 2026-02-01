// cli/impl/commands/export/export_command.cpp
#include "export_command.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"  // [新增]
#include "cli/impl/app/app_context.hpp"
#include "cli/impl/utils/arg_utils.hpp"
#include "domain/utils/time_utils.hpp"
#include "reports/shared/utils/format/iso_week_utils.hpp"
#include "reports/shared/utils/format/year_utils.hpp"

namespace {
auto ParseFormats(const ParsedArgs& args) -> std::vector<ReportFormat> {
  if (args.has("format")) {
    return ArgUtils::parse_report_formats(args.get("format"));
  }
  return {ReportFormat::Markdown};
}

auto RequiresExportArgument(const std::string& sub_command) -> bool {
  return sub_command == "day" || sub_command == "month" ||
         sub_command == "week" || sub_command == "year" ||
         sub_command == "recent" || sub_command == "all-recent";
}

auto ParseDaysList(const std::string& export_arg) -> std::vector<int> {
  std::vector<int> days_list;
  std::string token;
  std::istringstream token_stream(export_arg);
  while (std::getline(token_stream, token, ',')) {
    try {
      days_list.push_back(std::stoi(token));
    } catch (const std::exception&) {
      throw std::runtime_error(
          "Invalid number in days list for 'export all-recent': " + token);
    }
  }
  return days_list;
}

void RunExportWithArgument(IReportHandler& report_handler,
                           const std::string& sub_command,
                           const std::string& export_arg, ReportFormat format) {
  if (sub_command == "day") {
    const std::string date_str = normalize_to_date_format(export_arg);
    report_handler.run_export_single_day_report(date_str, format);
  } else if (sub_command == "month") {
    const std::string month_str = normalize_to_month_format(export_arg);
    report_handler.run_export_single_month_report(month_str, format);
  } else if (sub_command == "week") {
    IsoWeek week{};
    if (!parse_iso_week(export_arg, week)) {
      throw std::runtime_error(
          "Invalid ISO week format. Use YYYY-Www (e.g., 2026-W05).");
    }
    report_handler.run_export_single_week_report(format_iso_week(week), format);
  } else if (sub_command == "year") {
    int gregorian_year = 0;
    if (!parse_gregorian_year(export_arg, gregorian_year)) {
      throw std::runtime_error(
          "Invalid year format. Use Gregorian YYYY (e.g., 2026).");
    }
    report_handler.run_export_single_year_report(
        format_gregorian_year(gregorian_year), format);
  } else if (sub_command == "recent") {
    try {
      report_handler.run_export_single_period_report(std::stoi(export_arg),
                                                     format);
    } catch (const std::exception&) {
      throw std::runtime_error("Invalid number provided for 'export recent': " +
                               export_arg);
    }
  } else if (sub_command == "all-recent") {
    std::vector<int> days_list = ParseDaysList(export_arg);
    report_handler.run_export_all_period_reports_query(days_list, format);
  } else {
    throw std::runtime_error("Unknown export type '" + sub_command + "'.");
  }
}

void RunExportWithoutArgument(IReportHandler& report_handler,
                              const std::string& sub_command,
                              ReportFormat format) {
  if (sub_command == "all-day") {
    report_handler.run_export_all_daily_reports_query(format);
  } else if (sub_command == "all-month") {
    report_handler.run_export_all_monthly_reports_query(format);
  } else if (sub_command == "all-week") {
    report_handler.run_export_all_weekly_reports_query(format);
  } else if (sub_command == "all-year") {
    report_handler.run_export_all_yearly_reports_query(format);
  } else {
    throw std::runtime_error("Unknown export type '" + sub_command + "'.");
  }
}
}  // namespace

static CommandRegistrar<AppContext> registrar(
    "export", [](AppContext& ctx) -> std::unique_ptr<ExportCommand> {
      if (!ctx.report_handler) {
        throw std::runtime_error("ReportHandler not initialized");
      }
      return std::make_unique<ExportCommand>(*ctx.report_handler);
    });

ExportCommand::ExportCommand(IReportHandler& report_handler)
    : report_handler_(report_handler) {}

auto ExportCommand::get_definitions() const -> std::vector<ArgDef> {
  return {{"type",
           ArgType::Positional,
           {},
           "Export type (day, month, week, year, recent, all-day, all-month, "
           "all-week, all-year, all-recent...)",
           true,
           "",
           0},
          // argument 设为非必填，因为 all-daily 等不需要参数
          {"argument",
           ArgType::Positional,
           {},
           "Date, Month, Week (YYYY-Www), Year (YYYY), or Recent range",
           false,
           "",
           1},
          {"format",
           ArgType::Option,
           {"-f", "--format"},
           "Output format",
           false,
           "md"},
          {"output",
           ArgType::Option,
           {"-o", "--output"},
           "Output directory",
           false,
           ""},
          {"db",
           ArgType::Option,
           {"--db", "--database"},
           "Database path",
           false,
           ""}};
}

auto ExportCommand::get_help() const -> std::string {
  return "Exports reports (day, month, week, year, recent, etc.) to specified "
         "formats "
         "(md, tex, typ).";
}

void ExportCommand::execute(const CommandParser& parser) {
  // 1. 统一验证
  ParsedArgs args = CommandValidator::validate(parser, get_definitions());

  const std::string sub_command = args.get("type");
  const std::string export_arg = args.get("argument");  // 可能为空
  const std::vector<ReportFormat> formats = ParseFormats(args);

  // 2. 遍历所有请求的格式进行导出
  for (const auto& format : formats) {
    if (RequiresExportArgument(sub_command)) {
      if (export_arg.empty()) {
        throw std::runtime_error("Argument required for export type '" +
                                 sub_command + "'.");
      }
      RunExportWithArgument(report_handler_, sub_command, export_arg, format);
    } else {
      RunExportWithoutArgument(report_handler_, sub_command, format);
    }
  }
}
