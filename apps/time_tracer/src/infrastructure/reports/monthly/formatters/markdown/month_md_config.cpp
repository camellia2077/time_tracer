// infrastructure/reports/monthly/formatters/markdown/month_md_config.cpp
#include "infrastructure/reports/monthly/formatters/markdown/month_md_config.hpp"

#include "infrastructure/reports/shared/interfaces/formatter_c_string_view_utils.hpp"

MonthMdConfig::MonthMdConfig(const TtMonthMdConfigV1& config)
    : MonthBaseConfig(config.labels),
      project_breakdown_label_(formatter_c_string_view_utils::ToString(
          config.labels.projectBreakdownLabel,
          "labels.projectBreakdownLabel")) {}

auto MonthMdConfig::GetProjectBreakdownLabel() const -> const std::string& {
  return project_breakdown_label_;
}
