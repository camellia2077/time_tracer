// api/cli/impl/commands/export/export_command.cpp
#include "api/cli/impl/commands/export/export_command.hpp"

#include <array>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "api/cli/impl/utils/period_utils.hpp"

using namespace time_tracer::cli::impl::utils;

namespace {
constexpr std::string_view kSupportedExportTypes =
    "day, month, week, year, recent, all-day, all-month, all-week, all-year, "
    "all-recent";

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

struct NormalizedExportArgs {
  std::string_view kSubCommand;
  std::string_view kExportArg;
};

using ExportWithArgumentHandler = void (*)(IReportHandler&, std::string_view,
                                           ReportFormat);
using ExportWithoutArgumentHandler = void (*)(IReportHandler&, ReportFormat);

void ExportDay(IReportHandler& report_handler, std::string_view kExportArg,
               ReportFormat format) {
  report_handler.RunExportSingleDayReport(std::string(kExportArg), format);
}

void ExportMonth(IReportHandler& report_handler, std::string_view kExportArg,
                 ReportFormat format) {
  report_handler.RunExportSingleMonthReport(std::string(kExportArg), format);
}

void ExportWeek(IReportHandler& report_handler, std::string_view kExportArg,
                ReportFormat format) {
  report_handler.RunExportSingleWeekReport(std::string(kExportArg), format);
}

void ExportYear(IReportHandler& report_handler, std::string_view export_arg,
                ReportFormat format) {
  report_handler.RunExportSingleYearReport(std::string(export_arg), format);
}

void ExportRecent(IReportHandler& report_handler, std::string_view export_arg,
                  ReportFormat format) {
  try {
    report_handler.RunExportSinglePeriodReport(
        std::stoi(std::string(export_arg)), format);
  } catch (const std::exception&) {
    throw std::runtime_error("Invalid number provided for 'export recent': " +
                             std::string(export_arg));
  }
}

void ExportAllRecent(IReportHandler& report_handler,
                     std::string_view export_arg, ReportFormat format) {
  std::vector<int> days_list =
      ArgUtils::ParseNumberList(std::string(export_arg));
  report_handler.RunExportAllPeriodReportsQuery(days_list, format);
}

void ExportAllDay(IReportHandler& report_handler, ReportFormat format) {
  report_handler.RunExportAllDailyReportsQuery(format);
}

void ExportAllMonth(IReportHandler& report_handler, ReportFormat format) {
  report_handler.RunExportAllMonthlyReportsQuery(format);
}

void ExportAllWeek(IReportHandler& report_handler, ReportFormat format) {
  report_handler.RunExportAllWeeklyReportsQuery(format);
}

void ExportAllYear(IReportHandler& report_handler, ReportFormat format) {
  report_handler.RunExportAllYearlyReportsQuery(format);
}

constexpr std::array<std::pair<std::string_view, ExportWithArgumentHandler>, 6>
    kExportWithArgDispatchTable = {{
        {"day", &ExportDay},
        {"month", &ExportMonth},
        {"week", &ExportWeek},
        {"year", &ExportYear},
        {"recent", &ExportRecent},
        {"all-recent", &ExportAllRecent},
    }};

constexpr std::array<std::pair<std::string_view, ExportWithoutArgumentHandler>,
                     4>
    kExportWithoutArgDispatchTable = {{
        {"all-day", &ExportAllDay},
        {"all-month", &ExportAllMonth},
        {"all-week", &ExportAllWeek},
        {"all-year", &ExportAllYear},
    }};

[[nodiscard]] auto FindExportWithArgumentHandler(std::string_view kSubCommand)
    -> ExportWithArgumentHandler {
  for (const auto& [name, kHandler] : kExportWithArgDispatchTable) {
    if (name == kSubCommand) {
      return kHandler;
    }
  }
  return nullptr;
}

[[nodiscard]] auto FindExportWithoutArgumentHandler(
    std::string_view kSubCommand) -> ExportWithoutArgumentHandler {
  for (const auto& [name, kHandler] : kExportWithoutArgDispatchTable) {
    if (name == kSubCommand) {
      return kHandler;
    }
  }
  return nullptr;
}

void RunExportWithArgument(IReportHandler& report_handler,
                           const NormalizedExportArgs& args,
                           ReportFormat format) {
  const auto kHandler = FindExportWithArgumentHandler(args.kSubCommand);
  if (kHandler == nullptr) {
    throw std::runtime_error(
        "Unknown export type '" + std::string(args.kSubCommand) +
        "'. Supported: " + std::string(kSupportedExportTypes) + ".");
  }
  kHandler(report_handler, args.kExportArg, format);
}

void RunExportWithoutArgument(IReportHandler& report_handler,
                              std::string_view kSubCommand,
                              ReportFormat format) {
  const auto kHandler = FindExportWithoutArgumentHandler(kSubCommand);
  if (kHandler == nullptr) {
    throw std::runtime_error(
        "Unknown export type '" + std::string(kSubCommand) +
        "'. Supported: " + std::string(kSupportedExportTypes) + ".");
  }
  kHandler(report_handler, format);
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
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kSubCommand = args.Get("type");
  const std::string kExportArg = args.Get("argument");
  const std::vector<ReportFormat> kFormats =
      ParseFormats(args, default_format_);

  for (const auto& format : kFormats) {
    const auto kWithArgHandler = FindExportWithArgumentHandler(kSubCommand);
    const auto kWithoutArgHandler =
        FindExportWithoutArgumentHandler(kSubCommand);

    if ((kWithArgHandler == nullptr) && (kWithoutArgHandler == nullptr)) {
      throw std::runtime_error(
          "Unknown export type '" + kSubCommand +
          "'. Supported: " + std::string(kSupportedExportTypes) + ".");
    }

    if (kWithArgHandler != nullptr) {
      if (kExportArg.empty()) {
        throw std::runtime_error("Argument required for export type '" +
                                 kSubCommand + "'.");
      }
      const std::string kNormalizedArg = PeriodParser::Normalize(
          {.period_type_ = kSubCommand, .period_arg_ = kExportArg});
      RunExportWithArgument(
          report_handler_,
          {.kSubCommand = kSubCommand, .kExportArg = kNormalizedArg}, format);
      continue;
    }

    RunExportWithoutArgument(report_handler_, kSubCommand, format);
  }
}
