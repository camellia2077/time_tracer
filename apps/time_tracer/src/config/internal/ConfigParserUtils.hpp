// config/internal/ConfigParserUtils.hpp
#ifndef CONFIG_PARSER_UTILS_HPP
#define CONFIG_PARSER_UTILS_HPP

#include <filesystem>
#include <nlohmann/json.hpp>
#include "common/config/AppConfig.hpp"

namespace ConfigParserUtils {

    /**
     * @brief 解析 System / General 部分的配置
     *包括 error_log, export_root, save_processed_output 等
     */
    void parse_system_settings(const nlohmann::json& j, const std::filesystem::path& exe_path, AppConfig& config);

    /**
     * @brief 解析 Converter / Pipeline 部分的配置
     * 包括 interval_config 和 initial_top_parents
     */
    void parse_pipeline_settings(const nlohmann::json& j, const std::filesystem::path& config_dir, AppConfig& config);

    /**
     * @brief 解析 Reports 路径配置部分
     * 读取 reports 节点下 typst, latex, markdown 的各种路径
     */
    void parse_report_paths(const nlohmann::json& j, const std::filesystem::path& config_dir, AppConfig& config);

} // namespace ConfigParserUtils

#endif // CONFIG_PARSER_UTILS_HPP