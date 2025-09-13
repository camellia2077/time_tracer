// common/AppConfig.hpp
#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

// 应用程序的核心配置，由 config.json 加载
struct AppConfig {
    std::string interval_processor_config_path;
    std::string error_log_path;
    std::optional<std::string> export_path; // 用于存储自定义导出根目录路径
    std::unordered_map<std::string, std::string> initial_top_parents;// [新增] 用于存储 top_parents 映射的成员变量
};

// 文件处理操作的选项
struct AppOptions {
    std::string input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    bool enable_day_count_check = false;
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