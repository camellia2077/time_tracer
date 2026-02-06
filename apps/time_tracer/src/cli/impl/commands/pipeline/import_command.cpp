// cli/impl/commands/pipeline/import_command.cpp
#include "cli/impl/commands/pipeline/import_command.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>

#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp"  // [新增]
#include "cli/impl/app/app_context.hpp"

static CommandRegistrar<AppContext> registrar(
    "import", [](AppContext& ctx) -> std::unique_ptr<ImportCommand> {
      if (!ctx.workflow_handler) {
        throw std::runtime_error("WorkflowHandler not initialized");
      }
      return std::make_unique<ImportCommand>(*ctx.workflow_handler);
    });

ImportCommand::ImportCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

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
  return "Imports processed JSON data into the database.";
}

void ImportCommand::Execute(const CommandParser& parser) {
  ParsedArgs args = CommandValidator::Validate(parser, GetDefinitions());

  const std::string kInputPath = args.Get("path");

  std::cout << "You are about to import JSON data from: " << kInputPath
            << std::endl;
  std::cout << "Please ensure the data has been validated." << std::endl;
  workflow_handler_.RunDatabaseImport(kInputPath);
}
