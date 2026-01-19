#include "validate_source_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_validator.hpp" // [新增]
#include "common/app_options.hpp"
#include "cli/framework/core/command_registry.hpp" 
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <memory>

static CommandRegistrar<AppContext> registrar("validate-source", [](AppContext& ctx) {
    if (!ctx.workflow_handler) throw std::runtime_error("WorkflowHandler not initialized");
    return std::make_unique<ValidateSourceCommand>(*ctx.workflow_handler);
});

ValidateSourceCommand::ValidateSourceCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

std::vector<ArgDef> ValidateSourceCommand::get_definitions() const {
    return {
        {"path", ArgType::Positional, {}, "Source file path", true, "", 0}
    };
}

std::string ValidateSourceCommand::get_help() const {
    return "Validates the structure and format of source files (read-only).";
}

void ValidateSourceCommand::execute(const CommandParser& parser) {
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());
    
    AppOptions options;
    options.validate_source = true;
    options.convert = true; 
    options.validate_output = true;
    options.save_processed_output = false;
    options.date_check_mode = DateCheckMode::Continuity; 
    
    workflow_handler_.run_converter(args.get("path"), options);
}