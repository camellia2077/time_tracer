// config_validator/reprocessing/pipelines/DurationRules.hpp
#ifndef DURATION_RULES_CONFIG_VALIDATOR_HPP
#define DURATION_RULES_CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

class DurationRules {
public:
    bool validate(const nlohmann::json& duration_json);
};

#endif // DURATION_RULES_CONFIG_VALIDATOR_HPP