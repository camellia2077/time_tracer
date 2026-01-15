// reports/monthly/formatters/latex/MonthTexConfig.cpp
#include "MonthTexConfig.hpp"

// [修改] 接收 toml::table，并透传给基类和样式配置
MonthTexConfig::MonthTexConfig(const toml::table& config)
    : MonthBaseConfig(config),
      style_(config)
{
}