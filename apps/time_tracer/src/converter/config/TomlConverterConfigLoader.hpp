// converter/config/TomlConverterConfigLoader.hpp
#ifndef CONVERTER_CONFIG_TOML_CONVERTER_CONFIG_LOADER_HPP_
#define CONVERTER_CONFIG_TOML_CONVERTER_CONFIG_LOADER_HPP_

#include "common/config/IConfigLoader.hpp"
#include "converter/config/ConverterConfig.hpp"
#include <toml++/toml.h>

class TomlConverterConfigLoader : public IConfigLoader<ConverterConfig> {
public:
    explicit TomlConverterConfigLoader(const toml::table& config_table);
    bool load(ConverterConfig& config_object) override;

private:
    const toml::table& toml_source_;
};

#endif // CONVERTER_CONFIG_TOML_CONVERTER_CONFIG_LOADER_HPP_