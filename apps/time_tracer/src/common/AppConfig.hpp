// common/AppConfig.hpp
#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include <unordered_map>

// [核心修改] 引入包含 DateCheckMode 定义的头文件
#include "converter/validator/common/ValidatorUtils.hpp"

namespace fs = std::filesystem;

// [重构] 报告模块相关的配置路径
// 将原本扁平的 config_path 字段按功能分组
struct ReportConfig {
    // Tex configs
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

// [重构] 转换与流水线核心处理模块的配置
struct PipelineConfig {
    // 间隔处理器（IntervalProcessor）的配置文件路径
    fs::path interval_processor_config_path;
    // 初始的顶级父节点映射关系
    std::unordered_map<fs::path, fs::path> initial_top_parents;
};

// 应用程序的核心配置，由 config.json 加载
struct AppConfig {
    // --- 全局基础环境设置 ---
    fs::path exe_dir_path;
    fs::path error_log_path;
    // 全局导出路径（可选）
    std::optional<fs::path> export_path;

    // --- 子模块配置 ---
    PipelineConfig pipeline; // 转换流水线配置
    ReportConfig reports;    // 报告生成配置
};

// 文件处理操作的选项
// (保持原样，这部分属于运行时参数，而非静态环境配置)
struct AppOptions {
    fs::path input_path;
    bool run_all = false;
    bool convert = false;
    bool validate_source = false;
    bool validate_output = false;
    
    // [核心修改] 新增枚举类型的日期检查模式，默认为 None (不检查)
    DateCheckMode date_check_mode = DateCheckMode::None;
    
    // [新增] 是否将转换后的 JSON 保存到磁盘
    bool save_processed_output = false;
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