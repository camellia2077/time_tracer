// cli/commands/pipeline/Import.cpp
#include "Import.hpp"
#include "cli/CommandRegistry.hpp"
#include "cli/InputPrompter.hpp" 
#include <stdexcept>
#include <iostream>

static CommandRegistrar registrar("import", [](CliContext& ctx) {
    return std::make_unique<Import>(*ctx.workflow_handler);
});

Import::Import(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Import::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument (containing JSON files).");
    }

    const std::string& input_path = args[2];

    std::cout << "You are about to import JSON data from: " << input_path << std::endl;
    std::cout << "Please ensure the data has been validated." << std::endl;

    // 交互式确认，防止误操作
    InputPrompter prompter;
    if (!prompter.prompt_confirmation("Are you sure you want to write to the database?")) {
        std::cout << "\033[31mOperation cancelled.\033[0m" << std::endl;
        return;
    }
    
    std::cout << std::endl;
    
    // 调用 WorkflowHandler 执行核心逻辑
    workflow_handler_.run_database_import(input_path);
}