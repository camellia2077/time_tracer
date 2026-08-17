// infra/insights/daily/common/daily_formatter_base_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_DAILY_COMMON_DAILY_FORMATTER_BASE_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_DAILY_COMMON_DAILY_FORMATTER_BASE_CONFIG_H_

#include <string>
#include <vector>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/shared/api/shared_api.hpp"

struct StatisticItemConfig {
  std::string label;
  std::string db_column;
  bool show = true;
  std::vector<StatisticItemConfig> sub_items;
};

DISABLE_C4251_WARNING

class INSIGHTS_SHARED_API DailyFormatterBaseConfig {
 public:
  DailyFormatterBaseConfig(
      const DailyInsightsLabels& labels,
      const std::vector<InsightsStatisticsItem>& statistics_items);
  virtual ~DailyFormatterBaseConfig() = default;

  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetActivityCountLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusCountUnit() const -> const std::string&;
  [[nodiscard]] auto GetCustomSectionLabel() const -> const std::string&;
  [[nodiscard]] auto GetSummarySectionLabel() const -> const std::string&;
  [[nodiscard]] auto GetPeriodLabel() const -> const std::string&;
  [[nodiscard]] auto GetGetupTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetRemarkLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecords() const -> const std::string&;
  [[nodiscard]] auto GetStatisticsLabel() const -> const std::string&;
  [[nodiscard]] auto GetAllActivitiesLabel() const -> const std::string&;
  [[nodiscard]] auto GetActivityRemarkLabel() const -> const std::string&;
  [[nodiscard]] auto GetActivityConnector() const -> const std::string&;
  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatisticsItems() const
      -> const std::vector<StatisticItemConfig>&;

 protected:
  static auto BuildStatisticsItems(
      const std::vector<InsightsStatisticsItem>& statistics_items)
      -> std::vector<StatisticItemConfig>;

 private:
  void LoadBaseConfig(const DailyInsightsLabels& labels);

  std::string total_time_label_;
  std::string activity_count_label_;
  std::string status_count_unit_;
  std::string custom_section_label_;
  std::string summary_section_label_;
  std::string period_label_;
  std::string getup_time_label_;
  std::string remark_label_;
  std::string no_records_;
  std::string statistics_label_;
  std::string all_activities_label_;
  std::string activity_remark_label_;
  std::string activity_connector_;
  std::string project_breakdown_label_;

  std::vector<StatisticItemConfig> statistics_items_;
};

ENABLE_C4251_WARNING

#endif  // INFRASTRUCTURE_INSIGHTS_DAILY_COMMON_DAILY_FORMATTER_BASE_CONFIG_H_
