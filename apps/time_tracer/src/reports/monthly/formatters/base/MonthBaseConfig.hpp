// reports/monthly/formatters/base/MonthBaseConfig.hpp
#ifndef MONTH_BASE_CONFIG_HPP
#define MONTH_BASE_CONFIG_HPP

#include "reports/shared/api/shared_api.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"
#include <filesystem>

DISABLE_C4251_WARNING

class REPORTS_SHARED_API MonthBaseConfig {
public:
    explicit MonthBaseConfig(const std::filesystem::path& config_path);
    virtual ~MonthBaseConfig() = default;

    // --- 通用配置项的 Getters ---
    const std::string& get_report_title() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_format_message() const;

    const std::string& get_project_breakdown_label() const;

protected:
    nlohmann::json config_json_;

private:
    void load_base_config();

    std::string report_title_;
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string no_records_message_;
    std::string invalid_format_message_;
    std::string project_breakdown_label_;
};

ENABLE_C4251_WARNING

#endif // MONTH_BASE_CONFIG_HPP