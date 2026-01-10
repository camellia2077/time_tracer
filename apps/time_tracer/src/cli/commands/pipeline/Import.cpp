// cli/commands/pipeline/Import.cpp
#include "Import.hpp"
#include "cli/CommandRegistry.hpp"
#include "cli/InputPrompter.hpp" // [新增] 引入交互工具
#include <stdexcept>
#include <iostream>

static CommandRegistrar registrar("import", [](CliContext& ctx) {
    return std::make_unique<Import>(*ctx.workflow_handler);
});

Import::Import(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Import::execute(const CliParser& parser) {
    // 目前仍强制要求提供路径参数
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    }

    std::cout << "Now inserting into the database." << std::endl;
    std::cout << "Please ensure the data has been converted and validated." << std::endl;

    // [修改] 使用 InputPrompter 进行交互
    InputPrompter prompter;
    if (!prompter.prompt_confirmation("Are you sure you want to continue?")) {
        std::cout << "\033[31m\nOperation cancelled.\033[0m" << std::endl;
        return;
    }
    
    std::cout << std::endl;
    
    workflow_handler_.run_database_import(parser.get_filtered_args()[2]);
}