// reports/range/formatters/markdown/range_md_config.hpp
#ifndef REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
#define REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_

#include <toml++/toml.h>

#include "reports/range/common/range_base_config.hpp"

class RangeMdConfig : public RangeBaseConfig {
 public:
  explicit RangeMdConfig(const toml::table& config);
};

#endif  // REPORTS_RANGE_FORMATTERS_MARKDOWN_RANGE_MD_CONFIG_H_
