// infrastructure/config/internal/config_parser_utils.hpp
#ifndef INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
#define INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_

#include <toml++/toml.h>

#include <filesystem>

#include "infrastructure/config/models/app_config.hpp"

namespace ConfigParserUtils {

/**
 * @brief 解析 System / General 部分的配置 (TOML)
 */
void ParseSystemSettings(const toml::table& tbl,
                         const std::filesystem::path& exe_path,
                         const std::filesystem::path& source_config_path,
                         AppConfig& config);

/**
 * @brief 返回 `config/meta/bundle.toml` 的标准路径。
 */
auto ResolveBundlePath(const std::filesystem::path& config_dir)
    -> std::filesystem::path;

/**
 * @brief 解析 `config/meta/bundle.toml` 中的路径配置。
 * @return

 * * true 表示成功加载 bundle；false 表示 bundle 文件缺失。
 *
 *
 * @throws std::runtime_error 当 bundle 文件存在但无效。
 */
auto TryParseBundlePaths(const std::filesystem::path& config_dir,
                         AppConfig& config) -> bool;

/**
 * @brief 解析 CLI 默认参数与命令级配置 (TOML)
 */
void ParseCliDefaults(const toml::table& tbl,
                      const std::filesystem::path& exe_path,
                      const std::filesystem::path& source_config_path,
                      AppConfig& config);

}  // namespace ConfigParserUtils

#endif  // INFRASTRUCTURE_CONFIG_INTERNAL_CONFIG_PARSER_UTILS_H_
