// cli/commands/pipeline/Run.cpp
#include "Run.hpp"
#include <stdexcept>

Run::Run(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void Run::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }

    // [核心修改] 获取用户指定的检查模式，传递给 FileHandler
    // 默认为 None (由 Parser 决定)，不再强制 Full
    DateCheckMode mode = parser.get_date_check_mode();
    
    file_handler_.run_full_pipeline_and_import(args[2], mode);
}

std::string Run::get_help() const {
    return "run-pipeline <path> [--date-check <mode>]\t Run full pipeline. Mode: continuity, full.";
}
