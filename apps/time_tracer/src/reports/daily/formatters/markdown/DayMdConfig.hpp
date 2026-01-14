// reports/daily/formatters/markdown/DayMdConfig.hpp
#ifndef DAY_MD_CONFIG_HPP
#define DAY_MD_CONFIG_HPP

#include "reports/daily/common/DayBaseConfig.hpp"
#include <nlohmann/json.hpp>

class DayMdConfig : public DayBaseConfig {
public:
    // [修改] 接收 JSON
    explicit DayMdConfig(const nlohmann::json& config);
};

#endif // DAY_MD_CONFIG_HPP