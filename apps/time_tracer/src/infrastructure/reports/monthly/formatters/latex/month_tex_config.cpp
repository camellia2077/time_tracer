// infrastructure/reports/monthly/formatters/latex/month_tex_config.cpp
#include "infrastructure/reports/monthly/formatters/latex/month_tex_config.hpp"

MonthTexConfig::MonthTexConfig(const TtMonthTexConfigV1& config)
    : MonthBaseConfig(config.labels), style_(config.style) {}
