// reports/daily/formatters/markdown/DayMdConfig.hpp
#ifndef REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_
#define REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_

#include "reports/daily/common/DayBaseConfig.hpp"
#include <toml++/toml.h>

class DayMdConfig : public DayBaseConfig {
public:
    // [修改] 接收 TOML
    explicit DayMdConfig(const toml::table& config);
};

#endif // REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_HPP_