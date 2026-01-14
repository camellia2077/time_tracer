// common/AppOptions.hpp
#ifndef APP_OPTIONS_HPP
#define APP_OPTIONS_HPP

#include <filesystem>
#include "converter/validator/common/ValidatorUtils.hpp" // 需要 DateCheckMode

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

#endif // APP_OPTIONS_HPP