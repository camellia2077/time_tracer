// api/cli/impl/commands/pipeline/ingest_command.cpp
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp" // [新增]
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/arg_utils.hpp" // [新增] 用于 DateCheckMode 解析
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"
#include "shared/types/exceptions.hpp"

class IngestCommand : public ICommand {
public:
  IngestCommand(ITracerCoreApi &core_api, DateCheckMode default_date_check_mode,
                bool default_save_processed_output);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "ingest", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
  DateCheckMode default_date_check_mode_;
  bool default_save_processed_output_;
};

namespace {

auto BuildCoreErrorMessage(std::string_view fallback,
                           const std::string &error_message) -> std::string {
  if (!error_message.empty()) {
    return error_message;
  }
  return std::string(fallback);
}

void EnsureOperationSuccess(
    const tracer_core::core::dto::OperationAck &response,
    std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw tracer_core::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

} // namespace

namespace tracer_core::cli::impl::commands {

void RegisterIngestCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "ingest", [](AppContext &ctx) -> std::unique_ptr<IngestCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        DateCheckMode default_date_check = ctx.config.default_date_check_mode;
        if (ctx.config.command_defaults.ingest_date_check_mode) {
          default_date_check =
              *ctx.config.command_defaults.ingest_date_check_mode;
        }

        bool default_save_processed = ctx.config.default_save_processed_output;
        if (ctx.config.command_defaults.ingest_save_processed_output) {
          default_save_processed =
              *ctx.config.command_defaults.ingest_save_processed_output;
        }

        return std::make_unique<IngestCommand>(
            *ctx.core_api, default_date_check, default_save_processed);
      });
}

} // namespace tracer_core::cli::impl::commands

IngestCommand::IngestCommand(ITracerCoreApi &core_api,
                             DateCheckMode default_date_check_mode,
                             bool default_save_processed_output)
    : core_api_(core_api), default_date_check_mode_(default_date_check_mode),
      default_save_processed_output_(default_save_processed_output) {}

auto IngestCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::kPositional, {}, "Source directory", true, "", 0},
          {"date_check",
           ArgType::kOption,
           {"--date-check"},
           "Date check mode",
           false,
           ""},
          {"no_date_check",
           ArgType::kFlag,
           {"--no-date-check"},
           "Disable date check",
           false,
           ""},
          {"save",
           ArgType::kFlag,
           {"--save-processed"},
           "Save processed JSON",
           false,
           ""},
          {"no_save",
           ArgType::kFlag,
           {"--no-save"},
           "Do not save processed JSON",
           false,
           ""}};
}
auto IngestCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "ingest", ICommand::GetHelp());
}

void IngestCommand::Execute(const CommandParser &parser) {
  // 1. 统一验证与提取
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kInputPath = args.Get("path");

  // 2. 解析逻辑参数
  DateCheckMode mode = default_date_check_mode_;

  if (args.Has("date_check")) {
    mode = ArgUtils::ParseDateCheckMode(args.Get("date_check"));
  } else if (args.Has("no_date_check")) {
    mode = DateCheckMode::kNone;
  }

  bool save_json = default_save_processed_output_;
  if (args.Has("no_save")) {
    save_json = false;
  } else if (args.Has("save")) {
    save_json = true;
  }

  // 3. 执行业务 use case
  tracer_core::core::dto::IngestRequest request;
  request.input_path = kInputPath;
  request.date_check_mode = mode;
  request.save_processed_output = save_json;
  const auto kResponse = core_api_.RunIngest(request);
  EnsureOperationSuccess(kResponse, "Ingest command failed.");
}
