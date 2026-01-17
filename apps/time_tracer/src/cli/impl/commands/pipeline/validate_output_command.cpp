// cli/impl/commands/pipeline/validate_output_command.cpp
#include "validate_output_command.hpp"
#include "common/AppOptions.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/impl/utils/arg_utils.hpp"
#include "cli/impl/app/app_context.hpp" 
#include <stdexcept>
#include <memory>

// [修改] <AppContext>
static CommandRegistrar<AppContext> registrar("validate-output", [](AppContext& ctx) {
    return std::make_unique<ValidateOutputCommand>(*ctx.workflow_handler);
});

ValidateOutputCommand::ValidateOutputCommand(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateOutputCommand::execute(const CommandParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;

    auto mode_str = parser.get_option({"--date-check"});

    if (mode_str) {
        options.date_check_mode = ArgUtils::parse_date_check_mode(*mode_str);
    } else {
        if (parser.has_flag({"--no-date-check"})) {
             options.date_check_mode = DateCheckMode::None;
        } else {
             options.date_check_mode = workflow_handler_.get_config().default_date_check_mode;
        }
    }

    workflow_handler_.run_converter(filtered_args[2], options);
}