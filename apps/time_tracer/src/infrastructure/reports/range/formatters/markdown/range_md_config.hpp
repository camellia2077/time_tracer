// infrastructure/reports/range/formatters/markdown/range_md_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_

#include "infrastructure/reports/range/common/range_base_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class RangeMdConfig : public RangeBaseConfig {
 public:
  explicit RangeMdConfig(const TtRangeMdConfigV1& config);
};

#endif  // INFRASTRUCTURE_REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
