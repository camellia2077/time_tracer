// reports/period/formatters/latex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"

// [修改] 接收 JSON
PeriodTexConfig::PeriodTexConfig(const nlohmann::json& config)
    : PeriodBaseConfig(config),
      style_(config)
{
}