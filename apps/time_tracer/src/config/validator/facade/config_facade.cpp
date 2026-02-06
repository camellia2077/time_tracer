// config/validator/facade/config_facade.cpp
#include "config/validator/facade/config_facade.hpp"

#include "config/validator/converter/facade/converter_facade.hpp"
#include "config/validator/plugins/facade/plugin_validator.hpp"
#include "config/validator/reports/facade/query_facade.hpp"

auto ConfigFacade::ValidateConverterConfigs(
    const toml::table& main_tbl, const toml::table& mappings_tbl,
    const toml::table& duration_rules_tbl) -> bool {
  return ConverterFacade::Validate(main_tbl, mappings_tbl, duration_rules_tbl);

}

auto ConfigFacade::ValidateQueryConfigs(
    const std::vector<std::pair<std::string, toml::table>>& query_configs)
    -> bool {
  return QueryFacade::Validate(query_configs);

}

auto ConfigFacade::ValidatePlugins(const std::filesystem::path& plugins_path)
    -> bool {
  // 逻辑保持不变...
  const std::vector<std::string> kExpectedPlugins = {
      "DayMdFormatter",   "DayTexFormatter",   "DayTypFormatter",
      "RangeMdFormatter", "RangeTexFormatter", "RangeTypFormatter"};

  bool plugins_ok = PluginValidator::Validate(plugins_path, kExpectedPlugins);

  std::filesystem::path bin_dir = plugins_path.parent_path();
  bool core_ok = PluginValidator::Validate(bin_dir, {"reports_shared"});


  return plugins_ok && core_ok;
}
