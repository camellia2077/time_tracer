// cli/commands/pipeline/Run.cpp
#include "Run.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

// [修改] 静态注册
static CommandRegistrar registrar("run-pipeline", [](CliContext& ctx) {
    // 假设 CliContext 已更新为 workflow_handler
    return std::make_unique<Run>(*ctx.workflow_handler);
});

// [修改] 构造函数实现
Run::Run(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Run::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }

    // 获取用户指定的检查模式
    DateCheckMode mode = parser.get_date_check_mode();
    
    // [修改] 调用 workflow_handler_
    workflow_handler_.run_full_pipeline_and_import(args[2], mode);
}

std::string Run::get_help() const {
    return "run-pipeline <path> [--date-check <mode>]\t Run full pipeline. Mode: continuity, full.";
}