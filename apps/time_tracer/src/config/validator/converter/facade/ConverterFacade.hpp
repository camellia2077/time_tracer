// config/validator/converter/facade/ConverterFacade.hpp
#ifndef CONVERTER_FACADE_HPP
#define CONVERTER_FACADE_HPP

#include <toml++/toml.h> // [修改]

class ConverterFacade {
public:
    // [修改] 参数类型改为 toml::table
    bool validate(
        const toml::table& main_tbl,
        const toml::table& mappings_tbl,
        const toml::table& duration_rules_tbl
    ) const;
};

#endif // CONVERTER_FACADE_HPP