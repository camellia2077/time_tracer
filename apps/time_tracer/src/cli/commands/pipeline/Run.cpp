// cli/commands/pipeline/Run.cpp
#include "Run.hpp"
#include "cli/CommandRegistry.hpp"
#include "common/AppOptions.hpp"
#include <stdexcept>
#include <iostream>


static CommandRegistrar registrar("ingest", [](CliContext& ctx) {
    return std::make_unique<Run>(*ctx.workflow_handler);
});

Run::Run(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Run::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Usage: ingest <source_directory>");
    }

    const std::string& input_path = args[2];

    // 配置项：默认开启连续性检查，默认保存 JSON
    // 如果需要通过命令行参数控制 (例如 --no-save), 可以在这里解析 parser.has_flag(...)
    DateCheckMode mode = DateCheckMode::Continuity;
    bool save_json = true; 

    // 调用全流程
    workflow_handler_.run_full_pipeline_and_import(input_path, mode, save_json);
}