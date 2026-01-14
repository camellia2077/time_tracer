// cli/commands/pipeline/Run.cpp
#include "Run.hpp"
#include "cli/CliParser.hpp"
#include "cli/CommandRegistry.hpp"
#include "core/WorkflowHandler.hpp"
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

    // 1. 获取检查模式 (CLI 优先 > Config)
    DateCheckMode mode;
    auto mode_opt = parser.get_date_check_mode();

    if (mode_opt.has_value()) {
        // 如果 CLI 显式指定了（包括指定为 None），则使用 CLI 的值
        mode = mode_opt.value();
    } else {
        // 如果 CLI 未指定，则回退到 Config 的默认值
        mode = workflow_handler_.get_config().default_date_check_mode;
    }
    
    // 2. 获取保存逻辑 (CLI > Config)
    bool cli_force_save = parser.should_save_processed();
    bool cli_force_no_save = parser.should_skip_save();
    bool config_save_default = workflow_handler_.get_config().default_save_processed_output;
    
    bool final_save_decision = config_save_default;
    if (cli_force_save) {
        final_save_decision = true;
    } else if (cli_force_no_save) {
        final_save_decision = false;
    }
    
    // 将最终决策传递给 workflow_handler
    workflow_handler_.run_full_pipeline_and_import(args[2], mode, final_save_decision);
}