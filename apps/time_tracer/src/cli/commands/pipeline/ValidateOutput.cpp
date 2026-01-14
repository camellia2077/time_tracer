// cli/commands/pipeline/ValidateOutput.cpp
#include "ValidateOutput.hpp"
#include "common/AppOptions.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

// 静态注册：使用 workflow_handler
static CommandRegistrar registrar("validate-output", [](CliContext& ctx) {
    return std::make_unique<ValidateOutput>(*ctx.workflow_handler);
});

ValidateOutput::ValidateOutput(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateOutput::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;

    // [修复] 正确处理 std::optional 返回值
    // 1. 获取 CLI 解析结果
    auto mode_opt = parser.get_date_check_mode();

    if (mode_opt.has_value()) {
        // 情况 A: 用户在命令行显式指定了模式 (包括显式指定为 None/Off)
        // 此时 CLI 优先级最高，直接使用用户指定的值，覆盖配置文件
        options.date_check_mode = mode_opt.value();
    } else {
        // 情况 B: 用户在命令行没有任何相关输入 (std::nullopt)
        // 此时回退使用 config.json 中的默认配置
        options.date_check_mode = workflow_handler_.get_config().default_date_check_mode;
    }

    workflow_handler_.run_converter(filtered_args[2], options);
}