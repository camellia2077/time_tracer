// time_master_cli/commands/ValidateSourceCommand.cpp
#include "ValidateSourceCommand.hpp"
#include "common/AppConfig.hpp"
#include <stdexcept>

ValidateSourceCommand::ValidateSourceCommand(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void ValidateSourceCommand::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'validate-source' requires exactly one path argument.");
    }
    AppOptions options;
    options.validate_source = true;
    file_handler_.run_preprocessing(parser.get_filtered_args()[2], options);
}