// reports/period/formatters/latex/period_tex_config.cpp
#include "period_tex_config.hpp"

namespace reporting {

PeriodTexConfig::PeriodTexConfig(const toml::table& config)
    : PeriodBaseConfig(config),
      style_(config)
{
}

} // namespace reporting