// cli/impl/commands/pipeline/validate_source_command.cpp
#include "validate_source_command.hpp"
#include "common/app_options.hpp"
#include "cli/framework/core/command_registry.hpp" 
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <memory>

static CommandRegistrar<AppContext> registrar("validate-source", [](AppContext& ctx) {
    return std::make_unique<ValidateSourceCommand>(*ctx.workflow_handler);
});

ValidateSourceCommand::ValidateSourceCommand(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateSourceCommand::execute(const CommandParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-source' requires a path argument.");
    }
    
    AppOptions options;
    options.validate_source = true;
    options.convert = true;
    options.validate_output = true;
    options.save_processed_output = false;
    options.date_check_mode = DateCheckMode::Continuity; 
    
    workflow_handler_.run_converter(filtered_args[2], options);
}