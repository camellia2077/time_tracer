// time_master_cli/commands/pipeline/ValidateOutput.cpp
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

    for (size_t i = 3; i < filtered_args.size(); ++i) {
        if (filtered_args[i] == "-edc" || filtered_args[i] == "--enable-day-check") {
            options.enable_day_count_check = true;
        }
    }
    file_handler_.run_preprocessing(filtered_args[2], options);
}