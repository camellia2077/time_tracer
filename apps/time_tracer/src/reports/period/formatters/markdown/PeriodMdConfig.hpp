// reports/period/formatters/markdown/PeriodMdConfig.hpp
#ifndef PERIOD_MD_CONFIG_HPP
#define PERIOD_MD_CONFIG_HPP

#include "reports/period/common/PeriodBaseConfig.hpp"
#include <string>
#include <filesystem>

class PeriodMdConfig : public PeriodBaseConfig {
public:
    explicit PeriodMdConfig(const std::filesystem::path& config_path);

    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

#endif // PERIOD_MD_CONFIG_HPP