// common/AppConfig.hpp
#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <unordered_map>

// [核心修改] 引入包含 DateCheckMode 定义的头文件
#include "reprocessing/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

// 应用程序的核心配置，由 config.json 加载
struct AppConfig {
    fs::path exe_dir_path;

    fs::path interval_processor_config_path;
    fs::path error_log_path;
    std::optional<fs::path> export_path;
    std::unordered_map<fs::path, fs::path> initial_top_parents;
    
    // tex
    fs::path day_tex_config_path;
    fs::path month_tex_config_path;
    fs::path period_tex_config_path;

    // Typst report config paths
    fs::path day_typ_config_path;
    fs::path month_typ_config_path;
    fs::path period_typ_config_path;
    
    // Markdown report config paths
    fs::path day_md_config_path;
    fs::path month_md_config_path;
    fs::path period_md_config_path;
};

// 文件处理操作的选项
struct AppOptions {
    fs::path input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    // [核心修改] 新增枚举类型的日期检查模式，默认为 None (不检查)
    DateCheckMode date_check_mode = DateCheckMode::None;
};

// 用于记录各处理阶段耗时的结构体
struct ProcessingTimings {
    double validation_source_ms = 0.0;
    double conversion_ms = 0.0;
    double validation_output_ms = 0.0;
};

// 文件处理结果的摘要
struct ProcessingResult {
    bool success = true;
    ProcessingTimings timings;
};

#endif // APP_CONFIG_HPP