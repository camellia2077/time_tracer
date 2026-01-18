// cli/impl/commands/pipeline/convert_command.cpp
#include "convert_command.hpp"
#include "common/app_options.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <memory>

static CommandRegistrar<AppContext> registrar("convert", [](AppContext& ctx) {
    return std::make_unique<ConvertCommand>(*ctx.workflow_handler);
});

ConvertCommand::ConvertCommand(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ConvertCommand::execute(const CommandParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    
    AppOptions options;
    options.validate_source = true;
    options.convert = true;
    options.save_processed_output = true;
    options.validate_output = true;
    options.date_check_mode = DateCheckMode::Continuity;

    workflow_handler_.run_converter(parser.get_filtered_args()[2], options);
}