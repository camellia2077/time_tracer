// infrastructure/reports/range/common/range_base_config.hpp
#ifndef REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
#define REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "infrastructure/reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API RangeBaseConfig {
 public:
  explicit RangeBaseConfig(toml::table config);
  virtual ~RangeBaseConfig() = default;

  [[nodiscard]] auto GetTitleTemplate() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetActualDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetStatusDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetSleepDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetExerciseDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetCardioDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetAnaerobicDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecordsMessage() const -> const std::string&;
  [[nodiscard]] auto GetInvalidRangeMessage() const -> const std::string&;
  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 protected:
  toml::table config_table_;

 private:
  void LoadBaseConfig();

  std::string title_template_;
  std::string total_time_label_;
  std::string actual_days_label_;
  std::string status_days_label_;
  std::string sleep_days_label_;
  std::string exercise_days_label_;
  std::string cardio_days_label_;
  std::string anaerobic_days_label_;
  std::string no_records_message_;
  std::string invalid_range_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
