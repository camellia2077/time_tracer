// config_validator/facade/ConfigFacade.cpp
#include "ConfigFacade.hpp"
#include "config_validator/reprocessing/facade/ReprocFacade.hpp" // [修改]
#include "config_validator/queries/facade/QueryFacade.hpp"           // [修改]

using json = nlohmann::json;

bool ConfigFacade::validate_preprocessing_configs(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    // 调用 reprocessing 领域的 facade
    ReprocFacade reprocessing_validator;
    return reprocessing_validator.validate(main_json, mappings_json, duration_rules_json);
}

bool ConfigFacade::validate_query_configs(
    const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
) const {
    // 调用 queries 领域的 facade
    QueryFacade query_validator;
    return query_validator.validate(query_configs);
}