// time_master_cli/commands/ConvertCommand.cpp
#include "ConvertCommand.hpp"
#include "common/AppConfig.hpp"
#include <stdexcept>

ConvertCommand::ConvertCommand(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void ConvertCommand::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    AppOptions options;
    options.convert = true;
    file_handler_.run_preprocessing(parser.get_filtered_args()[2], options);
}