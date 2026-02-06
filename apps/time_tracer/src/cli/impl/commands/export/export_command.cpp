// cli/impl/commands/export/export_command.cpp
#include "cli/impl/commands/export/export_command.hpp"

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
auto ParseFormats(const ParsedArgs& args, const std::string& default_format)
    -> std::vector<ReportFormat> {
  if (args.Has("format")) {
    return ArgUtils::ParseReportFormats(args.Get("format"));
  }
  if (!default_format.empty()) {
    return ArgUtils::ParseReportFormats(default_format);
  }
  return {ReportFormat::kMarkdown};

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

struct NormalizedExportArgs {
  std::string_view sub_command;
  std::string_view export_arg;
};

void RunExportWithArgument(IReportHandler& report_handler,
                            const NormalizedExportArgs& args,
                            ReportFormat format) {
  if (args.sub_command == "day") {
    const std::string kDateStr =
        NormalizeToDateFormat(std::string(args.export_arg));
    report_handler.RunExportSingleDayReport(kDateStr, format);
  } else if (args.sub_command == "month") {
    const std::string kMonthStr =
        NormalizeToMonthFormat(std::string(args.export_arg));
    report_handler.RunExportSingleMonthReport(kMonthStr, format);
  } else if (args.sub_command == "week") {
    IsoWeek week{};
    if (!ParseIsoWeek(std::string(args.export_arg), week)) {
      throw std::runtime_error(
          "Invalid ISO week format. Use YYYY-Www (e.g., 2026-W05).");
    }
    report_handler.RunExportSingleWeekReport(FormatIsoWeek(week), format);
  } else if (args.sub_command == "year") {
    int gregorian_year = 0;
    if (!ParseGregorianYear(std::string(args.export_arg), gregorian_year)) {
      throw std::runtime_error(
          "Invalid year format. Use Gregorian YYYY (e.g., 2026).");
    }
    report_handler.RunExportSingleYearReport(
        FormatGregorianYear(gregorian_year), format);
  } else if (args.sub_command == "recent") {
    try {
      report_handler.RunExportSinglePeriodReport(
          std::stoi(std::string(args.export_arg)), format);
    } catch (const std::exception&) {
      throw std::runtime_error("Invalid number provided for 'export recent': " +
                               std::string(args.export_arg));
    }
  } else if (args.sub_command == "all-recent") {
    std::vector<int> days_list = ParseDaysList(std::string(args.export_arg));
    report_handler.RunExportAllPeriodReportsQuery(days_list, format);
  } else {
    throw std::runtime_error("Unknown export type '" +
                             std::string(args.sub_command) + "'.");
  }
}

void RunExportWithoutArgument(IReportHandler& report_handler,
                               const std::string& sub_command,
                               ReportFormat format) {
  if (sub_command == "all-day") {
    report_handler.RunExportAllDailyReportsQuery(format);
  } else if (sub_command == "all-month") {
    report_handler.RunExportAllMonthlyReportsQuery(format);
  } else if (sub_command == "all-week") {
    report_handler.RunExportAllWeeklyReportsQuery(format);
  } else if (sub_command == "all-year") {
    report_handler.RunExportAllYearlyReportsQuery(format);
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
      std::string format_value =
          ctx.config.command_defaults.export_format.value_or(
              ctx.config.defaults.default_format.value_or("md"));
      return std::make_unique<ExportCommand>(*ctx.report_handler, format_value);
    });

ExportCommand::ExportCommand(IReportHandler& report_handler,
                             std::string default_format)
    : report_handler_(report_handler),
      default_format_(std::move(default_format)) {}

auto ExportCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"type",
           ArgType::kPositional,
           {},
           "Export type (day, month, week, year, recent, all-day, all-month, "
           "all-week, all-year, all-recent...)",
           true,
           "",
           0},
          // argument 设为非必填，因为 all-daily 等不需要参数
          {"argument",
           ArgType::kPositional,
           {},
           "Date, Month, Week (YYYY-Www), Year (YYYY), or Recent range",
           false,
           "",
           1},
          {"format",
           ArgType::kOption,
           {"-f", "--format"},
           "Output format",
           false,
           ""},
          {"output",
           ArgType::kOption,
           {"-o", "--output"},
           "Output directory",
           false,
           ""},
          {"db",
           ArgType::kOption,
           {"--db", "--database"},
           "Database path",
           false,
           ""}};
}

auto ExportCommand::GetHelp() const -> std::string {
  return "Exports reports (day, month, week, year, recent, etc.) to specified "
         "formats "
         "(md, tex, typ).";
}

void ExportCommand::Execute(const CommandParser& parser) {
  // 1. 统一验证
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kSubCommand = args.Get("type");
  const std::string kExportArg = args.Get("argument");  // 可能为空
  const std::vector<ReportFormat> kFormats =
      ParseFormats(args, default_format_);

  // 2. 遍历所有请求的格式进行导出
  for (const auto& format : kFormats) {
    if (RequiresExportArgument(kSubCommand)) {
      if (kExportArg.empty()) {
        throw std::runtime_error("Argument required for export type '" +
                                 kSubCommand + "'.");
      }
      RunExportWithArgument(
          report_handler_,
          {.sub_command = kSubCommand, .export_arg = kExportArg}, format);
    } else {
      RunExportWithoutArgument(report_handler_, kSubCommand, format);
    }
  }
}
