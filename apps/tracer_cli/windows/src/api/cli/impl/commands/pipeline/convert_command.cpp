// api/cli/impl/commands/pipeline/convert_command.cpp
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
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "domain/types/date_check_mode.hpp"
#include "shared/types/exceptions.hpp"

class ConvertCommand : public ICommand {
public:
  ConvertCommand(ITracerCoreApi &core_api,
                 DateCheckMode default_date_check_mode,
                 bool default_save_processed_output,
                 bool default_validate_logic, bool default_validate_structure);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "convert", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
  DateCheckMode default_date_check_mode_;
  bool default_save_processed_output_;
  bool default_validate_logic_;
  bool default_validate_structure_;
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

void RegisterConvertCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "convert", [](AppContext &ctx) -> std::unique_ptr<ConvertCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        DateCheckMode default_date_check = ctx.config.default_date_check_mode;
        if (ctx.config.command_defaults.convert_date_check_mode) {
          default_date_check =
              *ctx.config.command_defaults.convert_date_check_mode;
        }

        bool default_save_processed = ctx.config.default_save_processed_output;
        if (ctx.config.command_defaults.convert_save_processed_output) {
          default_save_processed =
              *ctx.config.command_defaults.convert_save_processed_output;
        }

        bool default_validate_logic =
            ctx.config.command_defaults.convert_validate_logic.value_or(true);
        bool default_validate_structure =
            ctx.config.command_defaults.convert_validate_structure.value_or(
                true);

        return std::make_unique<ConvertCommand>(
            *ctx.core_api, default_date_check, default_save_processed,
            default_validate_logic, default_validate_structure);
      });
}

} // namespace tracer_core::cli::impl::commands

ConvertCommand::ConvertCommand(ITracerCoreApi &core_api,
                               DateCheckMode default_date_check_mode,
                               bool default_save_processed_output,
                               bool default_validate_logic,
                               bool default_validate_structure)
    : core_api_(core_api), default_date_check_mode_(default_date_check_mode),
      default_save_processed_output_(default_save_processed_output),
      default_validate_logic_(default_validate_logic),
      default_validate_structure_(default_validate_structure) {}

auto ConvertCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path", ArgType::kPositional, {}, "Source file path", true, "", 0}};
}
auto ConvertCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "convert", ICommand::GetHelp());
}

void ConvertCommand::Execute(const CommandParser &parser) {
  // 1. 统一验证
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  // 2. 构造 use case 请求 DTO 并执行
  tracer_core::core::dto::ConvertRequest request;
  request.input_path = args.Get("path");
  request.date_check_mode = default_date_check_mode_;
  request.save_processed_output = default_save_processed_output_;
  request.validate_logic = default_validate_logic_;
  request.validate_structure = default_validate_structure_;

  const auto kResponse = core_api_.RunConvert(request);
  EnsureOperationSuccess(kResponse, "Convert command failed.");
}
