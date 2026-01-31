// reports/range/formatters/latex/range_tex_config.cpp
#include "range_tex_config.hpp"

RangeTexConfig::RangeTexConfig(const toml::table& config)
    : RangeBaseConfig(config), style_(config) {}
