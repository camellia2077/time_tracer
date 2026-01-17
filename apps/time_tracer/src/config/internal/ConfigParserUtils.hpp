// config/internal/ConfigParserUtils.hpp
#ifndef CONFIG_INTERNAL_CONFIG_PARSER_UTILS_HPP_
#define CONFIG_INTERNAL_CONFIG_PARSER_UTILS_HPP_

#include <filesystem>
#include <toml++/toml.h> // 需要引入 toml++
#include "common/config/AppConfig.hpp"

namespace ConfigParserUtils {

    /**
     * @brief 解析 System / General 部分的配置 (TOML)
     */
    void parse_system_settings(const toml::table& tbl, const std::filesystem::path& exe_path, AppConfig& config);

    /**
     * @brief 解析 Converter / Pipeline 部分的配置 (TOML)
     */
    void parse_pipeline_settings(const toml::table& tbl, const std::filesystem::path& config_dir, AppConfig& config);

    /**
     * @brief 解析 Reports 路径配置部分 (TOML)
     */
    void parse_report_paths(const toml::table& tbl, const std::filesystem::path& config_dir, AppConfig& config);

} // namespace ConfigParserUtils

#endif // CONFIG_INTERNAL_CONFIG_PARSER_UTILS_HPP_