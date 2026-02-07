// infrastructure/reports/daily/formatters/markdown/day_md_config.cpp
#include "infrastructure/reports/daily/formatters/markdown/day_md_config.hpp"

// 透传 TOML 给基类
DayMdConfig::DayMdConfig(const toml::table& config) : DayBaseConfig(config) {}