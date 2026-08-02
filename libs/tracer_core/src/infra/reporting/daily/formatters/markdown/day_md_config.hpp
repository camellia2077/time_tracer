// infra/reporting/daily/formatters/markdown/day_md_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_

#include <string>

#include "infra/config/models/report_config_models.hpp"
#include "infra/reporting/daily/common/day_base_config.hpp"

class DayMdConfig : public DayBaseConfig {
 public:
  explicit DayMdConfig(const DailyMdConfig& config);

  [[nodiscard]] auto GetEndOnlyTimeFormat() const -> const std::string&;

 private:
  std::string end_only_time_format_;
};

#endif  // INFRASTRUCTURE_REPORTS_DAILY_FORMATTERS_MARKDOWN_DAY_MD_CONFIG_H_
