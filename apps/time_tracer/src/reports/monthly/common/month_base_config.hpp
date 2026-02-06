// reports/monthly/common/month_base_config.hpp
#ifndef REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_
#define REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API MonthBaseConfig {
 public:
  // [修改] 接收 toml::table
  explicit MonthBaseConfig(toml::table config);
  virtual ~MonthBaseConfig() = default;

  [[nodiscard]] auto GetReportTitle() const -> const std::string&;
  [[nodiscard]] auto GetActualDaysLabel() const -> const std::string&;
  [[nodiscard]] auto GetTotalTimeLabel() const -> const std::string&;
  [[nodiscard]] auto GetNoRecordsMessage() const -> const std::string&;
  [[nodiscard]] auto GetInvalidFormatMessage() const -> const std::string&;

  [[nodiscard]] auto GetProjectBreakdownLabel() const -> const std::string&;

 protected:
  // [修改] 存储 TOML table
  toml::table config_table_;

 private:
  void LoadBaseConfig();

  std::string report_title_;
  std::string actual_days_label_;
  std::string total_time_label_;
  std::string no_records_message_;
  std::string invalid_format_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // REPORTS_MONTHLY_COMMON_MONTH_BASE_CONFIG_H_