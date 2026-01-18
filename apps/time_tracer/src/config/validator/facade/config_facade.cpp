// config/validator/facade/config_facade.cpp
#include "config_facade.hpp"
#include "config/validator/converter/facade/converter_facade.hpp"
#include "config/validator/reports/facade/query_facade.hpp"
#include "config/validator/plugins/facade/plugin_validator.hpp"

bool ConfigFacade::validate_converter_configs(
    const toml::table& main_tbl,
    const toml::table& mappings_tbl,
    const toml::table& duration_rules_tbl
) const {
    ConverterFacade reprocessing_validator;
    return reprocessing_validator.validate(main_tbl, mappings_tbl, duration_rules_tbl);
}

bool ConfigFacade::validate_query_configs(
    const std::vector<std::pair<std::string, toml::table>>& query_configs
) const {
    QueryFacade query_validator;
    return query_validator.validate(query_configs);
}

bool ConfigFacade::validate_plugins(const std::filesystem::path& plugins_path) const {
    // 逻辑保持不变...
    const std::vector<std::string> expected_plugins = {
        "DayMdFormatter", "DayTexFormatter", "DayTypFormatter",
        "MonthMdFormatter", "MonthTexFormatter", "MonthTypFormatter",
        "PeriodMdFormatter", "PeriodTexFormatter", "PeriodTypFormatter"
    };

    PluginValidator validator;
    bool plugins_ok = validator.validate(plugins_path, expected_plugins);

    std::filesystem::path bin_dir = plugins_path.parent_path();
    bool core_ok = validator.validate(bin_dir, {"reports_shared"});

    return plugins_ok && core_ok;
}