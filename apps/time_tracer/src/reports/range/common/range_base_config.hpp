// reports/range/common/range_base_config.hpp
#ifndef REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
#define REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_

#include <toml++/toml.h>

#include <string>

#include "reports/shared/api/shared_api.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API RangeBaseConfig {
 public:
  explicit RangeBaseConfig(toml::table config);
  virtual ~RangeBaseConfig() = default;

  const std::string& get_title_template() const;
  const std::string& get_total_time_label() const;
  const std::string& get_actual_days_label() const;
  const std::string& get_no_records_message() const;
  const std::string& get_invalid_range_message() const;
  const std::string& get_project_breakdown_label() const;

 protected:
  toml::table config_table_;

 private:
  void load_base_config();

  std::string title_template_;
  std::string total_time_label_;
  std::string actual_days_label_;
  std::string no_records_message_;
  std::string invalid_range_message_;
  std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif  // REPORTS_RANGE_COMMON_RANGE_BASE_CONFIG_H_
