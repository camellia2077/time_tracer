// infrastructure/reports/range/formatters/latex/range_tex_config.cpp
#include "infrastructure/reports/range/formatters/latex/range_tex_config.hpp"

RangeTexConfig::RangeTexConfig(const TtRangeTexConfigV1& config)
    : RangeBaseConfig(config.labels), style_(config.style) {}
