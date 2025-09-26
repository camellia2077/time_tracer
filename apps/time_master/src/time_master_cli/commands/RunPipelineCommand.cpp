// time_master_cli/commands/RunPipelineCommand.cpp
#include "RunPipelineCommand.hpp"
#include <stdexcept>

RunPipelineCommand::RunPipelineCommand(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void RunPipelineCommand::execute(const CliParser& parser) {
    const auto& args = parser.get_filtered_args();
    if (args.size() != 3) {
        throw std::runtime_error("Command 'run-pipeline' requires exactly one source directory path argument.");
    }
    file_handler_.run_full_pipeline_and_import(args[2]);
}

std::string RunPipelineCommand::get_help() const {
    return "run-pipeline <path>\t\t Run full pipeline: validate, convert, validate output, and import to database.";
}