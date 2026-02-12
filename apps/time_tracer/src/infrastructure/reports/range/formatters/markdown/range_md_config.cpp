// infrastructure/reports/range/formatters/markdown/range_md_config.cpp
#include "infrastructure/reports/range/formatters/markdown/range_md_config.hpp"

RangeMdConfig::RangeMdConfig(const TtRangeMdConfigV1& config)
    : RangeBaseConfig(config.labels) {}
