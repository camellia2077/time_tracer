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
    
    // [修改] 调用 workflow_handler_
    workflow_handler_.run_preprocessing(parser.get_filtered_args()[2], options);
}