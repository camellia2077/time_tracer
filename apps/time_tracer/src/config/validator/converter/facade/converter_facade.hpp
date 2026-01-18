// config/validator/converter/facade/converter_facade.hpp
#ifndef CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_HPP_
#define CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_HPP_

#include <toml++/toml.h> 

class ConverterFacade {
public:
    bool validate(
        const toml::table& main_tbl,
        const toml::table& mappings_tbl,
        const toml::table& duration_rules_tbl
    ) const;
};

#endif // CONFIG_VALIDATOR_CONVERTER_FACADE_CONVERTER_FACADE_HPP_