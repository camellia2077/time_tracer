// cli/commands/pipeline/ValidateSource.cpp
#include "ValidateSource.hpp"
#include "common/AppOptions.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

// [修复] 注册正确的命令字符串 "validate-source" 和类 ValidateSource
static CommandRegistrar registrar("validate-source", [](CliContext& ctx) {
    return std::make_unique<ValidateSource>(*ctx.workflow_handler);
});

// [修复] 实现 ValidateSource 的构造函数
ValidateSource::ValidateSource(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

// [修复] 实现 ValidateSource 的 execute 方法
void ValidateSource::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-source' requires a path argument.");
    }
    
    AppOptions options;
    // 设置只运行源文件验证
    options.validate_source = true;

    // 注意：validate-source 通常只检查 .txt 格式，不需要 date_check_mode（那是针对 JSON 输出的）
    
    workflow_handler_.run_converter(filtered_args[2], options);
}