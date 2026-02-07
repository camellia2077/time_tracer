// infrastructure/config/internal/config_parser_utils.hpp
#ifndef CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
#define CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_

#include <toml++/toml.h>

#include <filesystem>

#include "infrastructure/config/models/app_config.hpp"

namespace ConfigParserUtils {

/**
 * @brief 解析 System / General 部分的配置 (TOML)
 */
void ParseSystemSettings(const toml::table& tbl,
                         const std::filesystem::path& exe_path,
                         AppConfig& config);

/**
 * @brief 解析 Converter / Pipeline 部分的配置 (TOML)
 */
void ParsePipelineSettings(const toml::table& tbl,
                           const std::filesystem::path& config_dir,
                           AppConfig& config);

/**
 * @brief 解析 Reports 路径配置部分 (TOML)
 */
void ParseReportPaths(const toml::table& tbl,
                      const std::filesystem::path& config_dir,
                      AppConfig& config);

/**
 * @brief 解析 CLI 默认参数与命令级配置 (TOML)
 */
void ParseCliDefaults(const toml::table& tbl,
                      const std::filesystem::path& exe_path, AppConfig& config);

}  // namespace ConfigParserUtils

#endif  // CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
