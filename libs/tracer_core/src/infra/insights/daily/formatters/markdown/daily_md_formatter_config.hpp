// infra/insights/daily/formatters/markdown/daily_md_formatter_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAILY_MD_FORMATTER_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAILY_MD_FORMATTER_CONFIG_H_

#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/daily/common/daily_formatter_base_config.hpp"

class DailyMdFormatterConfig : public DailyFormatterBaseConfig {
 public:
  explicit DailyMdFormatterConfig(const DailyMdConfig& config);

  [[nodiscard]] auto GetEndOnlyTimeFormat() const -> const std::string&;

 private:
  std::string end_only_time_format_;
};

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_FORMATTERS_MARKDOWN_DAILY_MD_FORMATTER_CONFIG_H_
