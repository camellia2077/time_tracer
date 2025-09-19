// config_validator/queries/facade/QueryValidatorFacade.cpp
#include "QueryValidatorFacade.hpp"
#include "../pipelines/QueryConfigValidator.hpp"

#include <iostream>

bool QueryValidatorFacade::validate(
    const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
) const {
    QueryConfigValidator query_validator;
    bool all_ok = true;

    if (query_configs.empty()) {
        std::cerr << "[Validator] Warning: No query configuration files were provided for validation." << std::endl;
        return true;
    }

    for (const auto& config_pair : query_configs) {
        if (!query_validator.validate(config_pair.second, config_pair.first)) {
            all_ok = false;
        }
    }

    if(all_ok) {
        std::cout << "[Validator] All query configuration files are valid." << std::endl;
    }

    return all_ok;
}