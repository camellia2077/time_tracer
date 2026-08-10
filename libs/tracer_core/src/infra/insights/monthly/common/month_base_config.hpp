// infra/insights/monthly/common/month_base_config.hpp
#ifndef INFRASTRUCTURE_INSIGHTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_
#define INFRASTRUCTURE_INSIGHTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_

#include <string>

#include "infra/config/models/insights_config_models.hpp"
#include "infra/insights/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class INSIGHTS_SHARED_API MonthBaseConfig {
 public:
  explicit MonthBaseConfig(const MonthlyInsightsLabels& labels);
  virtual ~MonthBaseConfig() = default;

  [[nodiscard]] auto GetInsightsTitle() const -> const std::string&;
  [[nodiscard]] auto GetTitleTemplate() const -> const std::string&;
  [[nodiscard]] auto GetActualDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetExerciseDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetCardioDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetAnaerobicDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetActivityCountLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecordsMessage() const -> const std::string&;
  [[nodiscard]] auto GetInvalidFormatMessage() const -> const std::string&;
  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 private:
  void LoadBaseConfig(const MonthlyInsightsLabels& labels);

  std::string insights_title_;
  std::string title_template_;
  std::string actual_days_label_;
  std::string status_days_label_;
  std::string exercise_days_label_;
  std::string cardio_days_label_;
  std::string anaerobic_days_label_;
  std::string total_time_label_;
  std::string activity_count_label_;
  std::string no_records_message_;
  std::string invalid_format_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // INFRASTRUCTURE_INSIGHTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_
