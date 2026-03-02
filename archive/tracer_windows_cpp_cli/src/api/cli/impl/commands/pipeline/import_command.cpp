// api/cli/impl/commands/pipeline/import_command.cpp
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
#include "shared/types/exceptions.hpp"

class ImportCommand : public ICommand {
public:
  explicit ImportCommand(ITracerCoreApi &core_api);

  [[nodiscard]] auto GetDefinitions() const -> std::vector<ArgDef> override;
  [[nodiscard]] auto GetHelp() const -> std::string override;
  [[nodiscard]] auto GetCategory() const -> std::string override {
    return tracer_core::cli::framework::core::ResolveCommandCategory(
        "import", ICommand::GetCategory());
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

void RegisterImportCommand() {
  CommandRegistry<AppContext>::Instance().RegisterCommand(
      "import", [](AppContext &ctx) -> std::unique_ptr<ImportCommand> {
        if (!ctx.core_api) {
          throw std::runtime_error("Core API not initialized");
        }
        return std::make_unique<ImportCommand>(*ctx.core_api);
      });
}

} // namespace tracer_core::cli::impl::commands

ImportCommand::ImportCommand(ITracerCoreApi &core_api) : core_api_(core_api) {}

auto ImportCommand::GetDefinitions() const -> std::vector<ArgDef> {
  return {{"path",
           ArgType::kPositional,
           {},
           "Directory path containing JSON files",
           true,
           "",
           0}};
}

auto ImportCommand::GetHelp() const -> std::string {
  return tracer_core::cli::framework::core::ResolveCommandSummary(
      "import", ICommand::GetHelp());
}

void ImportCommand::Execute(const CommandParser &parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kInputPath = args.Get("path");
  const auto kResponse = core_api_.RunImport({.processed_path = kInputPath});
  EnsureOperationSuccess(kResponse, "Import command failed.");
}
