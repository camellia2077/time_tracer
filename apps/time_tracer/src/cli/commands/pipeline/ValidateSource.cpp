// cli/commands/pipeline/ValidateSource.cpp
#include "ValidateSource.hpp"
// [修复] AppOptions 现在独立在 common/AppOptions.hpp
#include "common/AppOptions.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

// [修改] 静态注册：使用 workflow_handler
static CommandRegistrar registrar("validate-source", [](CliContext& ctx) {
    // 确保 CliContext 已更新成员变量名为 workflow_handler
    return std::make_unique<ValidateSource>(*ctx.workflow_handler);
});

// [修改] 构造函数
ValidateSource::ValidateSource(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateSource::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'validate-source' requires exactly one path argument.");
    }
    AppOptions options;
    options.validate_source = true;
    
    workflow_handler_.run_converter(parser.get_filtered_args()[2], options);
}