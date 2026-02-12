// infrastructure/reports/daily/formatters/markdown/day_md_config.cpp
#include "infrastructure/reports/daily/formatters/markdown/day_md_config.hpp"

DayMdConfig::DayMdConfig(const TtDayMdConfigV1& config)
    : DayBaseConfig(config.labels, config.statisticsItems,
                    config.statisticsItemCount) {}
