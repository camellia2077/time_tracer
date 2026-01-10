// cli/commands/pipeline/Import.cpp
#include "Import.hpp"
#include "cli/CommandRegistry.hpp"
#include <stdexcept>
#include <iostream>

// [修改] 静态注册：使用 workflow_handler
static CommandRegistrar registrar("import", [](CliContext& ctx) {
    return std::make_unique<Import>(*ctx.workflow_handler);
});

// [修改] 构造函数
Import::Import(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Import::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'import' requires exactly one directory path argument.");
    }

    std::cout << "Now inserting into the database." << std::endl;
    std::cout << "Please ensure the data has been converted and validated." << std::endl;
    std::cout << "Are you sure you want to continue? (y/n): ";

    char confirmation;
    std::cin >> confirmation;
    if (confirmation != 'y' && confirmation != 'Y') {
        std::cout << "\033[31m\nOperation cancelled.\033[0m" << std::endl;
        return;
    }
    std::cout << std::endl;
    
    // [修改] 调用 workflow_handler_
    workflow_handler_.run_database_import(parser.get_filtered_args()[2]);
}