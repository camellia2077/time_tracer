// cli/commands/pipeline/ValidateOutput.cpp
#include "ValidateOutput.hpp"
#include "common/AppConfig.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

// [修改] 静态注册：使用 workflow_handler
static CommandRegistrar registrar("validate-output", [](CliContext& ctx) {
    return std::make_unique<ValidateOutput>(*ctx.workflow_handler);
});

// [修改] 构造函数
ValidateOutput::ValidateOutput(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateOutput::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;

    // 获取用户指定的检查模式
    options.date_check_mode = parser.get_date_check_mode();

    // [修改] 调用 workflow_handler_
    workflow_handler_.run_preprocessing(filtered_args[2], options);
}