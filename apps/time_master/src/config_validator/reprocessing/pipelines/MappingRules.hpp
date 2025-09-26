// config_validator/reprocessing/pipelines/MappingRules.hpp
#ifndef MAPPINGS_CONFIG_VALIDATOR_HPP
#define MAPPINGS_CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

class MappingRules {
public:
    bool validate(const nlohmann::json& mappings_json);
};

#endif // MAPPINGS_CONFIG_VALIDATOR_HPP