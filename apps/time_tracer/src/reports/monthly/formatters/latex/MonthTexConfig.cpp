// reports/monthly/formatters/latex/MonthTexConfig.cpp
#include "MonthTexConfig.hpp"

MonthTexConfig::MonthTexConfig(const std::filesystem::path& config_path)
    : MonthBaseConfig(config_path),
      style_(config_json_) // [核心修改]
{
}