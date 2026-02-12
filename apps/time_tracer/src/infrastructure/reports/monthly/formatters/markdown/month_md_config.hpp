// infrastructure/reports/monthly/formatters/markdown/month_md_config.hpp
#ifndef REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_
#define REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_

#include <string>

#include "infrastructure/reports/monthly/common/month_base_config.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

class MonthMdConfig : public MonthBaseConfig {
 public:
  explicit MonthMdConfig(const TtMonthMdConfigV1& config);

  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 private:
  std::string project_breakdown_label_;
};

#endif  // REPORTS_MONTHLY_FORMATTERS_MARKDOWN_MONTH_MD_CONFIG_H_
