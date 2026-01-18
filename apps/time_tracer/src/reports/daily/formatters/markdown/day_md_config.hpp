// reports/daily/formatters/markdown/day_md_config.hpp
#ifndef REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_
#define REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_

#include "reports/daily/common/day_base_config.hpp"
#include <toml++/toml.h>

class DayMdConfig : public DayBaseConfig {
public:
    explicit DayMdConfig(const toml::table& config);
};

#endif // REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_