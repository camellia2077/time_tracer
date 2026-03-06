// infrastructure/reports/daily/formatters/markdown/day_md_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_

#include "infrastructure/config/models/report_config_models.hpp"
#include "infrastructure/reports/daily/common/day_base_config.hpp"

class DayMdConfig : public DayBaseConfig {
 public:
  explicit DayMdConfig(const DailyMdConfig& config);
};

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_
