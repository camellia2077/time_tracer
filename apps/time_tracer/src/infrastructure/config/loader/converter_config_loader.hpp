// infrastructure/config/loader/converter_config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_

#include <toml++/toml.h>

#include <filesystem>
#include <initializer_list>
#include <string_view>

#include "infrastructure/config/models/converter_config_models.hpp"

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
  static auto LoadFromFile(const std::filesystem::path& main_config_path)
      -> ConverterConfig;

 private:
  static auto MergeTomlTable(toml::table& target, const toml::table& source)
      -> void;
  static auto MergeSectionIfPresent(toml::table& main_tbl,
                                    const toml::table& source_tbl,
                                    std::string_view section_key) -> void;
  static auto MergeOptionalSections(
      toml::table& main_tbl, const std::filesystem::path& config_dir,
      std::string_view path_key,
      std::initializer_list<std::string_view> section_keys) -> void;
  static auto LoadMergedToml(const std::filesystem::path& main_config_path)
      -> toml::table;
  static auto ParseTomlToStruct(const toml::table& tbl,
                                ConverterConfig& out_config) -> void;

  static auto ParseBasicConfig(const toml::table& tbl, ConverterConfig& config)
      -> void;
  static auto ParseGeneratedActivities(const toml::table& tbl,
                                       ConverterConfig& config) -> void;
  static auto ParseMappings(const toml::table& tbl, ConverterConfig& config)
      -> void;
  static auto ParseDurationMappings(const toml::table& tbl,
                                    ConverterConfig& config) -> void;
};

#endif  // INFRASTRUCTURE_CONFIG_LOADER_CONVERTER_CONFIG_LOADER_H_
