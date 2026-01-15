// reports/daily/formatters/markdown/DayMdConfig.cpp
#include "DayMdConfig.hpp"

// 透传 TOML 给基类
DayMdConfig::DayMdConfig(const toml::table& config)
    : DayBaseConfig(config) {
}