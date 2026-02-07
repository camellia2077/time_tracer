// infrastructure/reports/monthly/formatters/markdown/month_md_config.cpp
#include "infrastructure/reports/monthly/formatters/markdown/month_md_config.hpp"

MonthMdConfig::MonthMdConfig(const toml::table& config)
    : MonthBaseConfig(config) {
  project_breakdown_label_ =
      config_table_["project_breakdown_label"].value_or<std::string>(
          "Project Breakdown");
}

auto MonthMdConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}