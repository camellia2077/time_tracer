// reports/period/common/PeriodBaseConfig.hpp
#ifndef PERIOD_BASE_CONFIG_HPP
#define PERIOD_BASE_CONFIG_HPP

#include "reports/shared/api/shared_api.hpp"
#include <string>
#include <nlohmann/json.hpp>
// [修改] 移除 IO
// #include "reports/shared/utils/config/ConfigUtils.hpp"

DISABLE_C4251_WARNING

class REPORTS_SHARED_API PeriodBaseConfig {
public:
    // [修改] 接收 JSON
    explicit PeriodBaseConfig(const nlohmann::json& config);
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
    nlohmann::json config_json_;

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

ENABLE_C4251_WARNING

#endif // PERIOD_BASE_CONFIG_HPP