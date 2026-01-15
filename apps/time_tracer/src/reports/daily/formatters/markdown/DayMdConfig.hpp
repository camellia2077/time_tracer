// reports/daily/formatters/markdown/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP

#include "reports/daily/common/DayBaseConfig.hpp"
#include <toml++/toml.h>

class DayMdConfig : public DayBaseConfig {
public:
    // [修改] 接收 TOML
    explicit DayMdConfig(const toml::table& config);
};

#endif // DAY_MD_CONFIG_HPP