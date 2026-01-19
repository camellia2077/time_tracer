// config/loader/converter_config_loader.hpp
#ifndef CONFIG_LOADER_CONVERTER_CONFIG_LOADER_HPP_
#define CONFIG_LOADER_CONVERTER_CONFIG_LOADER_HPP_

#include <filesystem>
#include <toml++/toml.h>
#include "common/config/models/converter_config_models.hpp"

/**
 * @class ConverterConfigLoader
 * @brief 负责加载、合并并解析 Converter 相关的 TOML 配置。
 */
class ConverterConfigLoader {
public:
    /**
     * @brief 从指定的主配置文件路径加载完整的 Converter 配置。
     * 自动处理 mappings_config_path 的合并。
     */
    static ConverterConfig load_from_file(const std::filesystem::path& main_config_path);

private:
    static void merge_toml_table(toml::table& target, const toml::table& source);
    static toml::table load_merged_toml(const std::filesystem::path& main_config_path);
    static void parse_toml_to_struct(const toml::table& tbl, ConverterConfig& out_config);
};

#endif // CONFIG_LOADER_CONVERTER_CONFIG_LOADER_HPP_