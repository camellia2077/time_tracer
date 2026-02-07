// infrastructure/reports/daily/formatters/markdown/day_md_config.hpp
#ifndef REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_
#define REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_

#include <toml++/toml.h>

#include "infrastructure/reports/daily/common/day_base_config.hpp"

class DayMdConfig : public DayBaseConfig {
 public:
  explicit DayMdConfig(const toml::table& config);
};

#endif  // REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_