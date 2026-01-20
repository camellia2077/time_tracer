// cli/impl/commands/pipeline/import_command.cpp
#include "import_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/core/command_validator.hpp" // [新增]
#include "cli/framework/io/console_input.hpp"
#include "cli/impl/app/app_context.hpp" 
#include <stdexcept>
#include <iostream>
#include <memory>

static CommandRegistrar<AppContext> registrar("import", [](AppContext& ctx) {
    if (!ctx.workflow_handler) throw std::runtime_error("WorkflowHandler not initialized");
    return std::make_unique<ImportCommand>(*ctx.workflow_handler);
});

ImportCommand::ImportCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

std::vector<ArgDef> ImportCommand::get_definitions() const {
    return {
        {"path", ArgType::Positional, {}, "Directory path containing JSON files", true, "", 0}
    };
}

std::string ImportCommand::get_help() const {
    return "Imports processed JSON data into the database.";
}

void ImportCommand::execute(const CommandParser& parser) {
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());

    const std::string input_path = args.get("path");

    std::cout << "You are about to import JSON data from: " << input_path << std::endl;
    std::cout << "Please ensure the data has been validated." << std::endl;

    ConsoleInput prompter; 
    if (!prompter.prompt_confirmation("Are you sure you want to write to the database?")) {
        std::cout << "\033[31mOperation cancelled.\033[0m" << std::endl;
        return;
    }
    
    std::cout << std::endl;
    workflow_handler_.run_database_import(input_path);
}