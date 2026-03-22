// infra/reports/range/formatters/markdown/range_md_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_

#include "infra/config/models/report_config_models.hpp"
#include "infra/reports/range/common/range_base_config.hpp"

class RangeMdConfig : public RangeBaseConfig {
 public:
  explicit RangeMdConfig(const RangeReportLabels& labels);
};

#endif  // INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
