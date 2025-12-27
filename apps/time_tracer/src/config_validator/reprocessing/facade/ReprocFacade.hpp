// config_validator/reprocessing/facade/ReprocFacade.hpp
#ifndef REPROC_FACADE_HPP
#define REPROC_FACADE_HPP

#include <nlohmann/json.hpp>

class ReprocFacade {
public:
    bool validate(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    ) const;
};

#endif // REPROC_FACADE_HPP