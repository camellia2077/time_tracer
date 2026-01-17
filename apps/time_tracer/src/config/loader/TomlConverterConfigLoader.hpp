// config/loader/TomlConverterConfigLoader.hpp
#ifndef CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_HPP_
#define CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_HPP_

#include "common/config/IConfigLoader.hpp"
#include "converter/config/ConverterConfig.hpp"
#include <toml++/toml.h>

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

#endif // CONFIG_LOADER_TOML_CONVERTER_CONFIG_LOADER_HPP_