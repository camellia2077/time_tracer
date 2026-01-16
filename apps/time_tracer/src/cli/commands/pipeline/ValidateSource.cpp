// cli/commands/pipeline/ValidateSource.cpp
#include "ValidateSource.hpp"
#include "common/AppOptions.hpp"
#include "cli/CommandRegistry.hpp" 
#include <stdexcept>

static CommandRegistrar registrar("validate-source", [](CliContext& ctx) {
    return std::make_unique<ValidateSource>(*ctx.workflow_handler);
});

ValidateSource::ValidateSource(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void ValidateSource::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-source' requires a path argument.");
    }
    
    AppOptions options;
    
    // 1. 开启源文件结构验证
    options.validate_source = true;
    
    // 2. 开启转换 (Text -> Struct)，以便进行后续逻辑验证
    options.convert = true;

    // 3. 开启输出逻辑验证 (Struct -> JSON -> Validate)
    options.validate_output = true;

    // 4. [关键] 强制关闭文件保存，确保全流程在内存中进行
    options.save_processed_output = false;

    // 5. 设置默认的日期检查模式 (Continuity 检查连续性)
    options.date_check_mode = DateCheckMode::Continuity; 
    
    workflow_handler_.run_converter(filtered_args[2], options);
}