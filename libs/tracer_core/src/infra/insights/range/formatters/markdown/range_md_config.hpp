// infra/insights/range/formatters/markdown/range_md_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/range/common/range_base_config.hpp"

class RangeMdConfig : public RangeBaseConfig {
 public:
  explicit RangeMdConfig(const RangeInsightsLabels& labels);
};

#endif  // INFRASTRUCTURE_INSIGHTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
