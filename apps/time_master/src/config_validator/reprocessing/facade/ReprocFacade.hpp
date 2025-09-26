// config_validator/reprocessing/facade/ReprocFacade.hpp
#ifndef REPROCESSING_VALIDATOR_FACADE_HPP
#define REPROCESSING_VALIDATOR_FACADE_HPP

#include <nlohmann/json.hpp>

class ReprocFacade {
public:
    bool validate(
        const nlohmann::json& main_json,
        const nlohmann::json& mappings_json,
        const nlohmann::json& duration_rules_json
    ) const;
};

#endif // REPROCESSING_VALIDATOR_FACADE_HPP