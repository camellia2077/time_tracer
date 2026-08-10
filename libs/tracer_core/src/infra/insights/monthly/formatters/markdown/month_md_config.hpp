// infra/insights/monthly/formatters/markdown/month_md_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_

#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/monthly/common/month_base_config.hpp"

class MonthMdConfig : public MonthBaseConfig {
 public:
  explicit MonthMdConfig(const MonthlyMdConfig& config);

  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 private:
  std::string project_breakdown_label_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_
