// common/config/AppConfig.hpp
#ifndef APP_CONFIG_HPP
#define APP_CONFIG_HPP

#include <string>
#include <filesystem>
#include <vector>
#include <optional>
#include <unordered_map>

namespace fs = std::filesystem;

struct ReportConfig {
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

struct PipelineConfig {
    fs::path interval_processor_config_path;
    std::unordered_map<fs::path, fs::path> initial_top_parents;
};

struct AppConfig {
    fs::path exe_dir_path;
    fs::path error_log_path;
    std::optional<fs::path> export_path;
    PipelineConfig pipeline;
    ReportConfig reports;
};

#endif // APP_CONFIG_HPP