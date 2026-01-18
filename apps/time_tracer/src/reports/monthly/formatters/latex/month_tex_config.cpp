// reports/monthly/formatters/latex/month_tex_config.cpp
#include "month_tex_config.hpp"

MonthTexConfig::MonthTexConfig(const toml::table& config)
    : MonthBaseConfig(config),
      style_(config)
{
}