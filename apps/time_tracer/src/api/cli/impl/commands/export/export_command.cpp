// api/cli/impl/commands/export/export_command.cpp
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "api/cli/impl/utils/period_utils.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "shared/types/exceptions.hpp"

class ExportCommand : public ICommand {
 public:
  ExportCommand(ITimeTracerCoreApi& core_api, std::string default_format);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return "Export";
  }

  auto Execute(const CommandParser& parser) -> void override;

 private:
  ITimeTracerCoreApi& core_api_;
  std::string default_format_;
};

using namespace time_tracer::cli::impl::utils;
using namespace time_tracer::core::dto;

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

[[nodiscard]] auto ParseExportType(std::string_view value)
    -> std::optional<ReportExportType> {
  if (value == "day") {
    return ReportExportType::kDay;
  }
  if (value == "month") {
    return ReportExportType::kMonth;
  }
  if (value == "recent") {
    return ReportExportType::kRecent;
  }
  if (value == "week") {
    return ReportExportType::kWeek;
  }
  if (value == "year") {
    return ReportExportType::kYear;
  }
  if (value == "all-day") {
    return ReportExportType::kAllDay;
  }
  if (value == "all-month") {
    return ReportExportType::kAllMonth;
  }
  if (value == "all-recent") {
    return ReportExportType::kAllRecent;
  }
  if (value == "all-week") {
    return ReportExportType::kAllWeek;
  }
  if (value == "all-year") {
    return ReportExportType::kAllYear;
  }
  return std::nullopt;
}

[[nodiscard]] auto RequiresArgument(const ReportExportType kType) -> bool {
  return kType == ReportExportType::kDay || kType == ReportExportType::kMonth ||
         kType == ReportExportType::kRecent ||
         kType == ReportExportType::kWeek || kType == ReportExportType::kYear ||
         kType == ReportExportType::kAllRecent;
}

auto BuildCoreErrorMessage(std::string_view fallback,
                           const std::string& error_message) -> std::string {
  if (!error_message.empty()) {
    return error_message;
  }
  return std::string(fallback);
}

void EnsureOperationSuccess(const OperationAck& response,
                            std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw time_tracer::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

}  // namespace

static CommandRegistrar<AppContext> registrar(
    "export", [](AppContext& ctx) -> std::unique_ptr<ExportCommand> {
      if (!ctx.core_api) {
        throw std::runtime_error("Core API not initialized");
      }
      std::string format_value =
          ctx.config.command_defaults.export_format.value_or(
              ctx.config.defaults.default_format.value_or("md"));
      return std::make_unique<ExportCommand>(*ctx.core_api, format_value);
    });

ExportCommand::ExportCommand(ITimeTracerCoreApi& core_api,
                             std::string default_format)
    : core_api_(core_api), default_format_(std::move(default_format)) {}

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

  const auto kExportType = ParseExportType(kSubCommand);
  if (!kExportType.has_value()) {
    throw std::runtime_error(
        "Unknown export type '" + kSubCommand +
        "'. Supported: " + std::string(kSupportedExportTypes) + ".");
  }

  const std::vector<ReportFormat> kFormats =
      ParseFormats(args, default_format_);

  for (const auto& format : kFormats) {
    ReportExportRequest request;
    request.type = *kExportType;
    request.format = format;

    if (RequiresArgument(request.type)) {
      if (kExportArg.empty()) {
        throw std::runtime_error("Argument required for export type '" +
                                 kSubCommand + "'.");
      }

      if (request.type == ReportExportType::kAllRecent) {
        request.recent_days_list = ArgUtils::ParseNumberList(kExportArg);
      } else {
        request.argument = PeriodParser::Normalize(
            {.period_type_ = kSubCommand, .period_arg_ = kExportArg});
      }
    }

    const auto kResponse = core_api_.RunReportExport(request);
    EnsureOperationSuccess(kResponse, "Export command failed.");
  }
}
