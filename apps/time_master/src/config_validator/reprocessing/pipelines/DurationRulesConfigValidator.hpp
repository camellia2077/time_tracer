// config_validator/reprocessing/pipelines/DurationRulesConfigValidator.hpp
#ifndef DURATION_RULES_CONFIG_VALIDATOR_HPP
#define DURATION_RULES_CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

class DurationRulesConfigValidator {
public:
    bool validate(const nlohmann::json& duration_json);
};

#endif // DURATION_RULES_CONFIG_VALIDATOR_HPP