// cli/commands/pipeline/Convert.cpp
#include "Convert.hpp"
#include "common/AppConfig.hpp"
#include "cli/CommandRegistry.hpp"
#include <stdexcept>

// [修改] 静态注册
static CommandRegistrar registrar("convert", [](CliContext& ctx) {
    return std::make_unique<Convert>(*ctx.workflow_handler);
});

// [修改] 构造函数实现
Convert::Convert(WorkflowHandler& workflow_handler)
    : workflow_handler_(workflow_handler) {}

void Convert::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    AppOptions options;
    options.convert = true;

    // 对于 convert 命令，"保存文件"是其核心使命，所以默认为 true。
    // 只有当用户显式不想要保存时（如果未来有这种需求），我们才考虑处理。
    // 目前逻辑：只要是 convert 命令，就落地文件。
    options.save_processed_output = true;
    
    // [修改] 调用 workflow_handler_
    workflow_handler_.run_converter(parser.get_filtered_args()[2], options);
}