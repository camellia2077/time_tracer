// config/loader/toml_converter_config_loader.hpp
#ifndef CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_H_
#define CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_H_

#include "common/config/i_config_loader.hpp"
// [Fix] 修改头文件路径：指向重构后的位置
#include <toml++/toml.h>

#include "common/config/models/converter_config_models.hpp"

/**
 * @brief 基于 TOML 的 Converter 配置加载器
 */
class TomlConverterConfigLoader : public IConfigLoader<ConverterConfig> {
 public:
  /**
   * @brief 构造函数
   * @param config_table 已经加载或合并好的 TOML Table
   */
  explicit TomlConverterConfigLoader(const toml::table& config_table);

  bool Load(ConverterConfig& config_object) override;

 private:
  void parse_header_order(ConverterConfig& config);
  void parse_wake_keywords(ConverterConfig& config);
  void parse_top_parent_mapping(ConverterConfig& config);
  void parse_text_mappings(ConverterConfig& config);
  void parse_text_duration_mappings(ConverterConfig& config);
  void parse_duration_mappings(ConverterConfig& config);

  const toml::table& toml_source_;
};

#endif  // CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_H_