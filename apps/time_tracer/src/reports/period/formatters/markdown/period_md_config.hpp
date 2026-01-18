// reports/period/formatters/markdown/period_md_config.hpp
#ifndef REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_
#define REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_

#include "reports/period/common/period_base_config.hpp"
#include <string>
#include <toml++/toml.h>

namespace reporting {

class PeriodMdConfig : public PeriodBaseConfig {
public:
    explicit PeriodMdConfig(const toml::table& config);

    const std::string& get_project_breakdown_label() const;

private:
    std::string project_breakdown_label_;
};

} // namespace reporting

#endif // REPORTS_PERIOD_FORMATTERS_MARKDOWN_PERIOD_MD_CONFIG_HPP_