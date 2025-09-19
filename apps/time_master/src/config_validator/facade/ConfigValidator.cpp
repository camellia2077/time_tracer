// config_validator/facade/ConfigValidator.cpp
#include "ConfigValidator.hpp"
#include "../reprocessing/facade/ReprocessingValidatorFacade.hpp" // [修改]
#include "../queries/facade/QueryValidatorFacade.hpp"           // [修改]

using json = nlohmann::json;

bool ConfigValidator::validate_preprocessing_configs(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    // 调用 reprocessing 领域的 facade
    ReprocessingValidatorFacade reprocessing_validator;
    return reprocessing_validator.validate(main_json, mappings_json, duration_rules_json);
}

bool ConfigValidator::validate_query_configs(
    const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
) const {
    // 调用 queries 领域的 facade
    QueryValidatorFacade query_validator;
    return query_validator.validate(query_configs);
}