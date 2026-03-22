// infra/reports/range/common/range_base_config.hpp
#ifndef INFRASTRUCTURE_REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
#define INFRASTRUCTURE_REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_

#include <string>

#include "infra/config/models/report_config_models.hpp"
#include "infra/reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API RangeBaseConfig {
 public:
  explicit RangeBaseConfig(const RangeReportLabels& labels);
  virtual ~RangeBaseConfig() = default;

  [[nodiscard]] auto GetTitleTemplate() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetActualDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetWakeAnchorDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetExerciseDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetCardioDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetAnaerobicDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecordsMessage() const -> const std::string&;
  [[nodiscard]] auto GetInvalidRangeMessage() const -> const std::string&;
  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 private:
  void LoadBaseConfig(const RangeReportLabels& labels);

  std::string title_template_;
  std::string total_time_label_;
  std::string actual_days_label_;
  std::string status_days_label_;
  std::string wake_anchor_days_label_;
  std::string exercise_days_label_;
  std::string cardio_days_label_;
  std::string anaerobic_days_label_;
  std::string no_records_message_;
  std::string invalid_range_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // INFRASTRUCTURE_REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
