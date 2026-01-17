// common/AppOptions.hpp
#ifndef COMMON_APP_OPTIONS_HPP_
#define COMMON_APP_OPTIONS_HPP_

#include <filesystem>
// [架构优化] 不再依赖 converter 层的 ValidatorUtils，改为依赖 common 层的类型定义
#include "common/types/DateCheckMode.hpp"

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