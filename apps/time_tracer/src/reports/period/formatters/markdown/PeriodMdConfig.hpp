// reports/period/formatters/markdown/PeriodMdConfig.hpp
#ifndef PERIOD_MD_CONFIG_HPP
#define PERIOD_MD_CONFIG_HPP

#include "reports/period/common/PeriodBaseConfig.hpp"
#include <string>
#include <nlohmann/json.hpp>

class PeriodMdConfig : public PeriodBaseConfig {
public:
    // [修改] 接收 JSON
    explicit PeriodMdConfig(const nlohmann::json& config);

    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // PERIOD_MD_CONFIG_HPP