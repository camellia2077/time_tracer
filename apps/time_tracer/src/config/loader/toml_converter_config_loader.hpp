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

  bool load(ConverterConfig& config_object) override;

 private:
  const toml::table& toml_source_;
};

#endif  // CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_H_