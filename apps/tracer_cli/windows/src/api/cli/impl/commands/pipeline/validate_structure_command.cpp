// api/cli/impl/commands/pipeline/validate_structure_command.cpp
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "api/cli/framework/core/command_catalog.hpp"
#include "api/cli/framework/core/command_parser.hpp"
#include "api/cli/framework/core/command_registry.hpp"
#include "api/cli/framework/core/command_validator.hpp"
#include "api/cli/framework/interfaces/i_command.hpp"
#include "api/cli/impl/app/app_context.hpp"
#include "application/dto/core_requests.hpp"
#include "application/use_cases/i_tracer_core_api.hpp"
#include "shared/types/exceptions.hpp"

class ValidateStructureCommand : public ICommand {
public:
  explicit ValidateStructureCommand(ITracerCoreApi &core_api);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "validate-structure", ICommand::GetCategory());
  }

  auto Execute(const CommandParser &parser) -> void override;

private:
  ITracerCoreApi &core_api_;
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

void RegisterValidateStructureCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "validate-structure",
      [](AppContext &ctx) -> std::unique_ptr<ValidateStructureCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        return std::make_unique<ValidateStructureCommand>(*ctx.core_api);
      });
}

} // namespace tracer_core::cli::impl::commands

ValidateStructureCommand::ValidateStructureCommand(ITracerCoreApi &core_api)
    : core_api_(core_api) {}

auto ValidateStructureCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path",
           ArgType::kPositional,
           {},
           "Source directory or file path",
           true,
           "",
           0}};
}

auto ValidateStructureCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "validate-structure", ICommand::GetHelp());
}

void ValidateStructureCommand::Execute(const CommandParser &parser) {
  auto args = CommandValidator::Validate(parser, GetDefinitions());

  const auto kResponse =
      core_api_.RunValidateStructure({.input_path = args.Get("path")});
  EnsureOperationSuccess(kResponse, "Validate-structure command failed.");
}
