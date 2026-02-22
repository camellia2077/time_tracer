// api/cli/impl/commands/pipeline/validate_logic_command.cpp
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "api/cli/impl/utils/arg_utils.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_time_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"
#include "shared/types/exceptions.hpp"

class ValidateLogicCommand : public ICommand {
public:
  ValidateLogicCommand(ITimeTracerCoreApi &core_api,
                       DateCheckMode default_date_check_mode);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return time_tracer::cli::framework::core::ResolveCommandCategory(
        "validate-logic", "Pipeline");
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITimeTracerCoreApi &core_api_;
  DateCheckMode default_date_check_mode_;
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
    const time_tracer::core::dto::OperationAck &response,
    std::string_view fallback_message) {
  if (response.ok) {
    return;
  }
  throw time_tracer::common::LogicError(
      BuildCoreErrorMessage(fallback_message, response.error_message));
}

} // namespace

namespace time_tracer::cli::impl::commands {

void RegisterValidateLogicCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "validate-logic",
      [](AppContext &ctx) -> std::unique_ptr<ValidateLogicCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        DateCheckMode default_date_check = ctx.config.default_date_check_mode;
        if (ctx.config.command_defaults.validate_logic_date_check_mode) {
          default_date_check =
              *ctx.config.command_defaults.validate_logic_date_check_mode;
        }

        return std::make_unique<ValidateLogicCommand>(*ctx.core_api,
                                                      default_date_check);
      });
}

} // namespace time_tracer::cli::impl::commands

ValidateLogicCommand::ValidateLogicCommand(
    ITimeTracerCoreApi &core_api, DateCheckMode default_date_check_mode)
    : core_api_(core_api), default_date_check_mode_(default_date_check_mode) {}

auto ValidateLogicCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {
      {"path", ArgType::kPositional, {}, "Source directory path", true, "", 0},
      {"date_check",
       ArgType::kOption,
       {"--date-check"},
       "Date check mode (none/continuity/full)",
       false,
       ""},
      {"no_date_check",
       ArgType::kFlag,
       {"--no-date-check"},
       "Disable date check",
       false,
       ""}};
}

auto ValidateLogicCommand::GetHelp() const -> std::string {
  return time_tracer::cli::framework::core::ResolveCommandSummary(
      "validate-logic",
      "Validate business logic (e.g., date continuity, sleep cycles). "
      "Requires conversion but won't save output.");
}

void ValidateLogicCommand::Execute(const CommandParser &parser) {
  auto args = CommandValidator::Validate(parser, GetDefinitions());

  DateCheckMode mode = default_date_check_mode_;

  // 解析日期检查模式
  if (args.Has("date_check")) {
    mode = ArgUtils::ParseDateCheckMode(args.Get("date_check"));
  } else if (args.Has("no_date_check")) {
    mode = DateCheckMode::kNone;
  }

  const auto kResponse = core_api_.RunValidateLogic(
      {.input_path = args.Get("path"), .date_check_mode = mode});
  EnsureOperationSuccess(kResponse, "Validate-logic command failed.");
}
