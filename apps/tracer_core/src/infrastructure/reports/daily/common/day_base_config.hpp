// infrastructure/reports/daily/common/day_base_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_DAILY_COMMON_DAY_BASE_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_DAILY_COMMON_DAY_BASE_CONFIG_H_

#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"
#include "infrastructure/reports/shared/api/shared_api.hpp"

struct StatisticItemConfig {
  std::string label;
  std::string db_column;
  bool show = true;
  std::vector<StatisticItemConfig> sub_items;
};

DISABLE_C4251_WARNING

class REPORTS_SHARED_API DayBaseConfig {
 public:
  DayBaseConfig(const DailyReportLabels& labels,
                const std::vector<ReportStatisticsItem>& statistics_items);
  virtual ~DayBaseConfig() = default;

  [[nodiscard]] auto GetTitlePrefix() const -> const std::string&;
  [[nodiscard]] auto GetDateLabel() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusLabel() const -> const std::string&;
  [[nodiscard]] auto GetSleepLabel() const -> const std::string&;
  [[nodiscard]] auto GetGetupTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetRemarkLabel() const -> const std::string&;
  [[nodiscard]] auto GetExerciseLabel() const -> const std::string&;
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
      const std::vector<ReportStatisticsItem>& statistics_items)
      -> std::vector<StatisticItemConfig>;

 private:
  void LoadBaseConfig(const DailyReportLabels& labels);

  std::string title_prefix_;
  std::string date_label_;
  std::string total_time_label_;
  std::string status_label_;
  std::string sleep_label_;
  std::string getup_time_label_;
  std::string remark_label_;
  std::string exercise_label_;
  std::string no_records_;
  std::string statistics_label_;
  std::string all_activities_label_;
  std::string activity_remark_label_;
  std::string activity_connector_;
  std::string project_breakdown_label_;

  std::vector<StatisticItemConfig> statistics_items_;
};

ENABLE_C4251_WARNING

#endif  // INFRASTRUCTURE_REPORTS_DAILY_COMMON_DAY_BASE_CONFIG_H_
