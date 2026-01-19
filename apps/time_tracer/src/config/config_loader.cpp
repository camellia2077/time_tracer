// config/config_loader.cpp
#include "config_loader.hpp"
#include <iostream>
#include <stdexcept>
#include <toml++/toml.h>

#include "io/core/file_system_helper.hpp"
#include "config/loader/report_config_loader.hpp"
#include "config/internal/config_parser_utils.hpp"
// [新增]
#include "config/loader/converter_config_loader.hpp" 

namespace fs = std::filesystem;

namespace {
    void load_detailed_reports(AppConfig& config) {
        // --- Typst ---
        if (!config.reports.day_typ_config_path.empty())
            config.loaded_reports.typst.day = ReportConfigLoader::loadDailyTypConfig(config.reports.day_typ_config_path);
        if (!config.reports.month_typ_config_path.empty())
            config.loaded_reports.typst.month = ReportConfigLoader::loadMonthlyTypConfig(config.reports.month_typ_config_path);
        if (!config.reports.period_typ_config_path.empty())
            config.loaded_reports.typst.period = ReportConfigLoader::loadPeriodTypConfig(config.reports.period_typ_config_path);

        // --- LaTeX ---
        if (!config.reports.day_tex_config_path.empty())
            config.loaded_reports.latex.day = ReportConfigLoader::loadDailyTexConfig(config.reports.day_tex_config_path);
        if (!config.reports.month_tex_config_path.empty())
            config.loaded_reports.latex.month = ReportConfigLoader::loadMonthlyTexConfig(config.reports.month_tex_config_path);
        if (!config.reports.period_tex_config_path.empty())
            config.loaded_reports.latex.period = ReportConfigLoader::loadPeriodTexConfig(config.reports.period_tex_config_path);

        // --- Markdown ---
        if (!config.reports.day_md_config_path.empty())
            config.loaded_reports.markdown.day = ReportConfigLoader::loadDailyMdConfig(config.reports.day_md_config_path);
        if (!config.reports.month_md_config_path.empty())
            config.loaded_reports.markdown.month = ReportConfigLoader::loadMonthlyMdConfig(config.reports.month_md_config_path);
        if (!config.reports.period_md_config_path.empty())
            config.loaded_reports.markdown.period = ReportConfigLoader::loadPeriodMdConfig(config.reports.period_md_config_path);
    }
}

ConfigLoader::ConfigLoader(const std::string& exe_path_str) {
    try {
        exe_path = fs::canonical(fs::path(exe_path_str)).parent_path();
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Failed to determine executable directory: " + std::string(e.what()));
    }

    config_dir_path = exe_path / CONFIG_DIR_NAME;
    main_config_path = config_dir_path / "config.toml";
}

std::string ConfigLoader::get_main_config_path() const {
    return main_config_path.string();
}

AppConfig ConfigLoader::load_configuration() {
    if (!FileSystemHelper::exists(main_config_path)) {
        throw std::runtime_error("Configuration file not found: " + main_config_path.string());
    }

    toml::table tbl;
    try {
        tbl = toml::parse_file(main_config_path.string());
    } catch (const toml::parse_error& err) {
        throw std::runtime_error("Failed to parse config.toml: " + std::string(err.description()));
    }

    AppConfig app_config;
    app_config.exe_dir_path = exe_path;

    // 1. 解析基础路径和设置
    ConfigParserUtils::parse_system_settings(tbl, exe_path, app_config);
    ConfigParserUtils::parse_pipeline_settings(tbl, config_dir_path, app_config);
    ConfigParserUtils::parse_report_paths(tbl, config_dir_path, app_config);

    // 2. [新增] 加载 Converter 配置 (文件合并 + 解析逻辑现在在此处执行)
    try {
        if (!app_config.pipeline.interval_processor_config_path.empty()) {
            app_config.pipeline.loaded_converter_config = 
                ConverterConfigLoader::load_from_file(app_config.pipeline.interval_processor_config_path);
            
            // 将 initial_top_parents 从 AppConfig 注入到 ConverterConfig 中
            // 确保 Core 模块不需要再手动处理这个合并
            for (const auto& [key, val] : app_config.pipeline.initial_top_parents) {
                app_config.pipeline.loaded_converter_config.initial_top_parents[key.string()] = val.string();
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load converter configuration: " + std::string(e.what()));
    }

    // 3. 加载报表配置
    try {
        load_detailed_reports(app_config);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load report configuration details: " + std::string(e.what()));
    }

    return app_config;
}