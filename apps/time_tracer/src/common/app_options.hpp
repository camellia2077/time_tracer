// common/app_options.hpp
#ifndef COMMON_APP_OPTIONS_HPP_
#define COMMON_APP_OPTIONS_HPP_

#include <filesystem>
#include "common/types/date_check_mode.hpp"

namespace fs = std::filesystem;

struct AppOptions {
    fs::path input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    DateCheckMode date_check_mode = DateCheckMode::None;
    bool save_processed_output = false;
};

#endif // COMMON_APP_OPTIONS_HPP_