// cli/commands/pipeline/Run.cpp
#include "Run.hpp"
#include "cli/CliParser.hpp"
#include "cli/CommandRegistry.hpp"
#include "action_handler/WorkflowHandler.hpp"
#include <iostream>
#include <stdexcept>

// 静态注册命令
static CommandRegistrar registrar("run-pipeline", [](CliContext& ctx) {
    return std::make_unique<Run>(*ctx.workflow_handler);
});

// 注册别名 "blink"
static CommandRegistrar registrar_blink("blink", [](CliContext& ctx) {
    return std::make_unique<Run>(*ctx.workflow_handler);
});

Run::Run(WorkflowHandler& workflow_handler) : workflow_handler_(workflow_handler) {}

void Run::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }

    // 获取用户指定的检查模式
    DateCheckMode mode = parser.get_date_check_mode();
    
    // 获取是否保存文件的参数
    bool save_processed = parser.should_save_processed();
    
    // 将参数传递给 workflow_handler
    workflow_handler_.run_full_pipeline_and_import(args[2], mode, save_processed);
}
