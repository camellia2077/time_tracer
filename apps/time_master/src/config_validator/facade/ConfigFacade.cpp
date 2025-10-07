// config_validator/facade/ConfigFacade.cpp
#include "ConfigFacade.hpp"
#include "config_validator/reprocessing/facade/ReprocFacade.hpp"
#include "config_validator/reports/facade/QueryFacade.hpp"
#include "config_validator/plugins/facade/PluginValidator.hpp" // [Corrected] Was missing

using json = nlohmann::json;

bool ConfigFacade::validate_preprocessing_configs(
    const json& main_json,
    const json& mappings_json,
    const json& duration_rules_json
) const {
    ReprocFacade reprocessing_validator;
    return reprocessing_validator.validate(main_json, mappings_json, duration_rules_json);
}

bool ConfigFacade::validate_query_configs(
    const std::vector<std::pair<std::string, nlohmann::json>>& query_configs
) const {
    QueryFacade query_validator;
    return query_validator.validate(query_configs);
}

bool ConfigFacade::validate_plugins(const std::filesystem::path& plugins_path) const {
    const std::vector<std::string> expected_plugins = {
        "DayMdFormatter"
    };

    PluginValidator validator;
    return validator.validate(plugins_path, expected_plugins);
}