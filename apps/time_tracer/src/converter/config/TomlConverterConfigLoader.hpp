// converter/config/TomlConverterConfigLoader.hpp
#ifndef TOML_CONVERTER_CONFIG_LOADER_HPP
#define TOML_CONVERTER_CONFIG_LOADER_HPP

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

#endif // TOML_CONVERTER_CONFIG_LOADER_HPP