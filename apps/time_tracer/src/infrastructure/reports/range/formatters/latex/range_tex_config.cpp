// infrastructure/reports/range/formatters/latex/range_tex_config.cpp
#include "infrastructure/reports/range/formatters/latex/range_tex_config.hpp"

RangeTexConfig::RangeTexConfig(const toml::table& config)
    : RangeBaseConfig(config), style_(config) {}
