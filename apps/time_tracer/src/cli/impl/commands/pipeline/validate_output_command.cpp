#include "validate_output_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_validator.hpp" // [新增]
#include "common/app_options.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/impl/utils/arg_utils.hpp"
#include "cli/impl/app/app_context.hpp" 
#include <stdexcept>
#include <memory>

static CommandRegistrar<AppContext> registrar("validate-output", [](AppContext& ctx) {
    if (!ctx.workflow_handler) throw std::runtime_error("WorkflowHandler not initialized");
    return std::make_unique<ValidateOutputCommand>(*ctx.workflow_handler);
});

ValidateOutputCommand::ValidateOutputCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

std::vector<ArgDef> ValidateOutputCommand::get_definitions() const {
    return {
        {"path", ArgType::Positional, {}, "JSON file path", true, "", 0},
        {"date_check", ArgType::Option, {"--date-check"}, "Date check mode", false, ""},
        {"no_date_check", ArgType::Flag, {"--no-date-check"}, "Disable date check", false, ""}
    };
}

std::string ValidateOutputCommand::get_help() const {
    return "Validates the logic (e.g. date continuity) of processed JSON files (read-only).";
}

void ValidateOutputCommand::execute(const CommandParser& parser) {
    // 1. 统一验证
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());

    AppOptions options;
    options.validate_output = true;

    // 2. 解析逻辑
    if (args.has("date_check")) {
        options.date_check_mode = ArgUtils::parse_date_check_mode(args.get("date_check"));
    } else {
        if (args.has("no_date_check")) {
             options.date_check_mode = DateCheckMode::None;
        } else {
             options.date_check_mode = workflow_handler_.get_config().default_date_check_mode;
        }
    }

    // 3. 执行
    workflow_handler_.run_converter(args.get("path"), options);
}