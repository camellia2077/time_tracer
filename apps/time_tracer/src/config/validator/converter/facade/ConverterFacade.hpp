// config/validator/converter/facade/ConverterFacade.hpp
#ifndef CONVERTER_FACADE_HPP
#define CONVERTER_FACADE_HPP

#include <nlohmann/json.hpp>

class ConverterFacade {
public:
    bool validate(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    ) const;
};

#endif // CONVERTER_FACADE_HPP