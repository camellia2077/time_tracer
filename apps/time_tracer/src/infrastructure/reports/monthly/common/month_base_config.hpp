// infrastructure/reports/monthly/common/month_base_config.hpp
#ifndef REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_
#define REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_

#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"
#include "infrastructure/reports/shared/interfaces/formatter_c_abi_v2.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API MonthBaseConfig {
 public:
  explicit MonthBaseConfig(const TtMonthLabelsConfigV1& labels);
  virtual ~MonthBaseConfig() = default;

  [[nodiscard]] auto GetReportTitle() const -> const std::string&;
  [[nodiscard]] auto GetTitleTemplate() const -> const std::string&;
  [[nodiscard]] auto GetActualDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetSleepDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetExerciseDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetCardioDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetAnaerobicDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecordsMessage() const -> const std::string&;
  [[nodiscard]] auto GetInvalidFormatMessage() const -> const std::string&;
  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 private:
  void LoadBaseConfig(const TtMonthLabelsConfigV1& labels);

  std::string report_title_;
  std::string title_template_;
  std::string actual_days_label_;
  std::string status_days_label_;
  std::string sleep_days_label_;
  std::string exercise_days_label_;
  std::string cardio_days_label_;
  std::string anaerobic_days_label_;
  std::string total_time_label_;
  std::string no_records_message_;
  std::string invalid_format_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_
