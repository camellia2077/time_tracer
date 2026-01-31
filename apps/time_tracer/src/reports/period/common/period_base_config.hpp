// reports/period/common/period_base_config.hpp
#ifndef REPORTS_PERIOD_COMMON_PERIOD_BASE_CONFIG_H_
#define REPORTS_PERIOD_COMMON_PERIOD_BASE_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

namespace reporting {

class REPORTS_SHARED_API PeriodBaseConfig {
 public:
  explicit PeriodBaseConfig(toml::table config);
  virtual ~PeriodBaseConfig() = default;

  const std::string& get_report_title_prefix() const;
  const std::string& get_report_title_days() const;
  const std::string& get_report_title_date_separator() const;

  const std::string& get_actual_days_label() const;
  const std::string& get_total_time_label() const;
  const std::string& get_no_records_message() const;
  const std::string& get_invalid_days_message() const;
  const std::string& get_project_breakdown_label() const;

 protected:
  toml::table config_table_;

 private:
  void load_base_config();

  std::string report_title_prefix_;
  std::string report_title_days_;
  std::string report_title_date_separator_;

  std::string actual_days_label_;
  std::string total_time_label_;
  std::string no_records_message_;
  std::string invalid_days_message_;
  std::string project_breakdown_label_;
};

}  // namespace reporting

ENABLE_C4251_WARNING

#endif  // REPORTS_PERIOD_COMMON_PERIOD_BASE_CONFIG_H_