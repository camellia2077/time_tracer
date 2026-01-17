// cli/impl/commands/pipeline/import_command.cpp
#include "import_command.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/framework/io/console_input.hpp"
#include "cli/impl/app/app_context.hpp" 
#include <stdexcept>
#include <iostream>
#include <memory>

static CommandRegistrar<AppContext> registrar("import", [](AppContext& ctx) {
    return std::make_unique<ImportCommand>(*ctx.workflow_handler);
});

ImportCommand::ImportCommand(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ImportCommand::execute(const CommandParser& parser) { // [Fix] CommandParser
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument (containing JSON files).");
    }

    const std::string& input_path = args[2];

    std::cout << "You are about to import JSON data from: " << input_path << std::endl;
    std::cout << "Please ensure the data has been validated." << std::endl;

    // [Fix] ConsoleInput (was Console_Input)
    ConsoleInput prompter; 
    if (!prompter.prompt_confirmation("Are you sure you want to write to the database?")) {
        std::cout << "\033[31mOperation cancelled.\033[0m" << std::endl;
        return;
    }
    
    std::cout << std::endl;
    workflow_handler_.run_database_import(input_path);
}