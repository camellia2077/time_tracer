// time_master_cli/commands/pipeline/Convert.cpp
#include "Convert.hpp"
#include "common/AppConfig.hpp"
#include <stdexcept>

Convert::Convert(FileHandler& file_handler)
    : file_handler_(file_handler) {}

void Convert::execute(const CliParser& parser) {
    if (parser.get_filtered_args().size() != 3) {
        throw std::runtime_error("Command 'convert' requires exactly one path argument.");
    }
    AppOptions options;
    options.convert = true;
    file_handler_.run_preprocessing(parser.get_filtered_args()[2], options);
}