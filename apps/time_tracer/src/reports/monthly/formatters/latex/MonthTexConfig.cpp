// reports/monthly/formatters/latex/MonthTexConfig.cpp
#include "MonthTexConfig.hpp"

// [修改] 接收 JSON
MonthTexConfig::MonthTexConfig(const nlohmann::json& config)
    : MonthBaseConfig(config),
      style_(config)
{
}