// config/config_loader.cpp
#include "config_loader.hpp"
#include <iostream>
#include <stdexcept>
#include <toml++/toml.h>

// 引入 IO 模块
#include "io/core/file_system_helper.hpp"

// 引入加载器和解析工具
#include "config/loader/report_config_loader.hpp"
#include "config/internal/config_parser_utils.hpp"

namespace fs = std::filesystem;

namespace {
    // 报表加载逻辑保持不变，因为它们仍然指向 JSON 文件
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
    // [修改] 更改为 config.toml
    main_config_path = config_dir_path / "config.toml";
}

std::string ConfigLoader::get_main_config_path() const {
    return main_config_path.string();
}

AppConfig ConfigLoader::load_configuration() {
    // 1. 检查文件
    if (!FileSystemHelper::exists(main_config_path)) {
        throw std::runtime_error("Configuration file not found: " + main_config_path.string());
    }

    // 2. 读取并解析 TOML
    toml::table tbl;
    try {
        tbl = toml::parse_file(main_config_path.string());
    } catch (const toml::parse_error& err) {
        throw std::runtime_error("Failed to parse config.toml: " + std::string(err.description()));
    }

    AppConfig app_config;
    app_config.exe_dir_path = exe_path;

    // 3. 解析基础结构 (使用 TOML 版的 ParserUtils)
    ConfigParserUtils::parse_system_settings(tbl, exe_path, app_config);
    ConfigParserUtils::parse_pipeline_settings(tbl, config_dir_path, app_config);
    ConfigParserUtils::parse_report_paths(tbl, config_dir_path, app_config);

    // 4. 加载报表详情 (内部仍加载 JSON，路径由 config.toml 提供)
    try {
        load_detailed_reports(app_config);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load report configuration details: " + std::string(e.what()));
    }

    return app_config;
}