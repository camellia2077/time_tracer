// reports/period/formatters/latex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"

// [修改] 接收 toml::table
PeriodTexConfig::PeriodTexConfig(const toml::table& config)
    : PeriodBaseConfig(config),
      style_(config)
{
}