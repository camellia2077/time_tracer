// common/config/AppConfig.hpp
#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <filesystem>
#include <vector>
#include <optional>
#include <unordered_map>
#include "common/types/DateCheckMode.hpp"
// [新增] 引入配置模型
#include "common/config/ReportConfigModels.hpp"

namespace fs = std::filesystem;

// [新增] 用于存储加载后的所有报表配置
struct LoadedReportConfigs {
    struct {
        DailyTexConfig day;
        MonthlyTexConfig month;
        PeriodTexConfig period;
    } latex;

    struct {
        DailyTypConfig day;
        MonthlyTypConfig month;
        PeriodTypConfig period;
    } typst;

    struct {
        DailyMdConfig day;
        MonthlyMdConfig month;
        PeriodMdConfig period;
    } markdown;
};

struct PipelineConfig {
    fs::path interval_processor_config_path;
    std::unordered_map<fs::path, fs::path> initial_top_parents;
};

struct ReportConfigPaths {
    // Typst Report Configs
    fs::path day_typ_config_path;
    fs::path month_typ_config_path;
    fs::path period_typ_config_path;

    // LaTeX Report Configs
    fs::path day_tex_config_path;
    fs::path month_tex_config_path;
    fs::path period_tex_config_path;

    // Markdown Report Configs
    fs::path day_md_config_path;
    fs::path month_md_config_path;
    fs::path period_md_config_path;
};

struct AppConfig {
    fs::path exe_dir_path;
    fs::path error_log_path;
    std::optional<fs::path> export_path;
    
    bool default_save_processed_output = false;
    DateCheckMode default_date_check_mode = DateCheckMode::None;

    PipelineConfig pipeline;
    ReportConfigPaths reports; // 只有路径
    
    // [新增] 运行时持有的已解析配置对象
    LoadedReportConfigs loaded_reports; 
};

#endif // APP_CONFIG_HPP