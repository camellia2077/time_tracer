// config_validator/facade/ConfigFacade.cpp
#include "ConfigFacade.hpp"
#include "config_validator/converter/facade/ReprocFacade.hpp"
#include "config_validator/reports/facade/QueryFacade.hpp"
#include "config_validator/plugins/facade/PluginValidator.hpp"

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
    // 1. 验证 9 个具体的格式化器插件 (保持不变)
    const std::vector<std::string> expected_plugins = {
        "DayMdFormatter", "DayTexFormatter", "DayTypFormatter",
        "MonthMdFormatter", "MonthTexFormatter", "MonthTypFormatter",
        "PeriodMdFormatter", "PeriodTexFormatter", "PeriodTypFormatter"
    };

    PluginValidator validator;
    bool plugins_ok = validator.validate(plugins_path, expected_plugins);

    // 2. [新增] 验证核心共享库 reports_shared.dll 是否在 exe 同级目录
    // plugins_path 的父目录通常就是 bin 目录
    std::filesystem::path bin_dir = plugins_path.parent_path();
    bool core_ok = validator.validate(bin_dir, {"reports_shared"});

    return plugins_ok && core_ok;
}