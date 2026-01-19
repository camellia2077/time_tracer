// cli/impl/commands/pipeline/run_pipeline_command.cpp
#include "run_pipeline_command.hpp"
#include "cli/framework/core/command_registry.hpp"
#include "common/app_options.hpp"
#include "cli/impl/app/app_context.hpp" 
#include <stdexcept>
#include <iostream>
#include <memory>

// [[maybe_unused]] 必须放在最前面
[[maybe_unused]] static CommandRegistrar<AppContext> registrar("ingest", [](AppContext& ctx) {
    return std::make_unique<RunPipelineCommand>(*ctx.workflow_handler);
});

RunPipelineCommand::RunPipelineCommand(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void RunPipelineCommand::execute(const CommandParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Usage: ingest <source_directory>");
    }

    const std::string& input_path = args[2];

    // 配置项：默认开启连续性检查，默认保存 JSON
    DateCheckMode mode = DateCheckMode::Continuity;
    bool save_json = true; 

    // 调用全流程
    workflow_handler_.run_full_pipeline_and_import(input_path, mode, save_json);
}