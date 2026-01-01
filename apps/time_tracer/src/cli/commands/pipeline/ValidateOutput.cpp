// cli/commands/pipeline/ValidateOutput.cpp
#include "ValidateOutput.hpp"
#include "common/AppConfig.hpp"
#include <stdexcept>

ValidateOutput::ValidateOutput(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void ValidateOutput::execute(const CliParser& parser) {
    const auto& filtered_args = parser.get_filtered_args();
    if (filtered_args.size() < 3) {
        throw std::runtime_error("Command 'validate-output' requires a path argument.");
    }
    AppOptions options;
    options.validate_output = true;

    // [核心修改] 使用新的参数解析逻辑
    // 旧的 -edc 逻辑被替换
    options.date_check_mode = parser.get_date_check_mode();

    file_handler_.run_preprocessing(filtered_args[2], options);
}
