// reports/period/formatters/latex/PeriodTexConfig.cpp
#include "PeriodTexConfig.hpp"

PeriodTexConfig::PeriodTexConfig(const std::filesystem::path& config_path)
    : PeriodBaseConfig(config_path),
      style_(config_json_)
{
}