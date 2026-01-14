// reports/daily/formatters/markdown/DayMdConfig.cpp
#include "DayMdConfig.hpp"

// 透传 JSON 给基类
DayMdConfig::DayMdConfig(const nlohmann::json& config)
    : DayBaseConfig(config) {
}