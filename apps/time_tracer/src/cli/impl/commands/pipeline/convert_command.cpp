#include "convert_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_validator.hpp" // [新增]
#include "common/app_options.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "cli/impl/app/app_context.hpp"
#include <stdexcept>
#include <memory>

static CommandRegistrar<AppContext> registrar("convert", [](AppContext& ctx) {
    if (!ctx.workflow_handler) throw std::runtime_error("WorkflowHandler not initialized");
    return std::make_unique<ConvertCommand>(*ctx.workflow_handler);
});

ConvertCommand::ConvertCommand(IWorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

std::vector<ArgDef> ConvertCommand::get_definitions() const {
    return {
        {"path", ArgType::Positional, {}, "Source file path", true, "", 0}
    };
}
std::string ConvertCommand::get_help() const {
    return "Converts source files (e.g., .txt) to processed JSON format.";
}

void ConvertCommand::execute(const CommandParser& parser) {
    // 1. 统一验证
    ParsedArgs args = CommandValidator::validate(parser, get_definitions());
    
    // 2. 配置选项
    AppOptions options;
    options.validate_source = true;
    options.convert = true;
    options.save_processed_output = true;
    options.validate_output = true;
    options.date_check_mode = DateCheckMode::Continuity;

    // 3. 执行
    workflow_handler_.run_converter(args.get("path"), options);
}