// config_validator/pipelines/MainConfigValidator.hpp
#ifndef MAIN_CONFIG_VALIDATOR_HPP
#define MAIN_CONFIG_VALIDATOR_HPP

#include <string>
#include <nlohmann/json.hpp>

class MainConfigValidator {
public:
    bool validate(const nlohmann::json& main_json, std::string& out_mappings_path, std::string& out_duration_rules_path);
};

#endif // MAIN_CONFIG_VALIDATOR_HPP