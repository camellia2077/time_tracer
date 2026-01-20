#include "validate_structure_command.hpp"
#include "cli/framework/core/command_parser.hpp"
#include "cli/framework/core/command_validator.hpp"
#include "common/app_options.hpp"
#include "core/pipeline/pipeline_manager.hpp" // 直接调用 Pipeline
#include "cli/framework/core/command_registry.hpp"
#include "cli/impl/app/app_context.hpp"
#include <iostream>

// 注册命令：validate-structure
static CommandRegistrar<AppContext> registrar("validate-structure", [](AppContext& ctx) {
    return std::make_unique<ValidateStructureCommand>(ctx.config, ctx.config.export_path.value_or("./"));
});

ValidateStructureCommand::ValidateStructureCommand(const AppConfig& config, const std::filesystem::path& output_root)
    : app_config_(config), output_root_(output_root) {}

std::vector<ArgDef> ValidateStructureCommand::get_definitions() const {
    return {
        {"path", ArgType::Positional, {}, "Source directory or file path", true, "", 0}
    };
}

std::string ValidateStructureCommand::get_help() const {
    return "Validates the syntax and structure of source TXT files (Read-only).";
}

void ValidateStructureCommand::execute(const CommandParser& parser) {
    auto args = CommandValidator::validate(parser, get_definitions());

    AppOptions options;
    options.input_path = args.get("path");
    
    // [关键] 只开启结构验证
    options.validate_structure = true;
    options.convert = false;
    options.validate_logic = false;
    options.save_processed_output = false;

    // 委托给 Core 执行
    core::pipeline::PipelineManager manager(app_config_, output_root_);
    manager.run(options.input_path.string(), options);
}