// cli/commands/pipeline/Convert.cpp
#include "Convert.hpp"
#include "common/AppOptions.hpp"
#include "cli/CommandRegistry.hpp"
#include <stdexcept>

// 静态注册
static CommandRegistrar registrar("convert", [](CliContext& ctx) {
    return std::make_unique<Convert>(*ctx.workflow_handler);
});

Convert::Convert(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Convert::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    
    AppOptions options;

    // [核心修改 1] 开启源文件验证
    // 只有这一步通过了，PipelineManager 才会继续往下走。
    // 如果验证失败，程序会提前终止，不会浪费算力去转换，也不会写入垃圾文件。
    options.validate_source = true;

    // [步骤 2] 开启转换
    options.convert = true;

    // [步骤 3] 强制保存 (Convert 命令的核心使命)
    // 这一步是在 ConverterStep 内部执行的。
    // 由于 Pipeline 是串行的，只有 validate_source 成功，才会进入 ConverterStep 并执行保存。
    options.save_processed_output = true;

    // [可选优化] 开启输出逻辑验证
    // 建议开启，这样用户不仅知道源文件格式对了，还能知道生成的逻辑（如睡眠时间）是否合理
    options.validate_output = true;
    options.date_check_mode = DateCheckMode::Continuity; // 为输出验证提供默认模式

    workflow_handler_.run_converter(parser.get_filtered_args()[2], options);
}