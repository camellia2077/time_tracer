// reports/monthly/formatters/base/MonthBaseConfig.hpp
#ifndef MONTH_BASE_CONFIG_HPP
#define MONTH_BASE_CONFIG_HPP

#include "reports/shared/shared_api.hpp" // <--- [新增] 1. 包含API宏
#include <string>
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"

DISABLE_C4251_WARNING // <--- [新增] 2. 禁用C4251警告 (因为有 std::string)

class REPORTS_SHARED_API MonthBaseConfig { // <--- [修改] 3. 添加API宏
public:
    explicit MonthBaseConfig(const std::string& config_path);
    virtual ~MonthBaseConfig() = default;

    // --- 通用配置项的 Getters ---
    const std::string& get_report_title() const;
    const std::string& get_actual_days_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_no_records_message() const;
    const std::string& get_invalid_format_message() const;

protected:
    nlohmann::json config_json_;

private:
    void load_base_config();

    // --- 共享的成员变量 ---
    std::string report_title_;
    std::string actual_days_label_;
    std::string total_time_label_;
    std::string no_records_message_;
    std::string invalid_format_message_;
};

ENABLE_C4251_WARNING // <--- [新增] 4. 恢复C4251警告

#endif // MONTH_BASE_CONFIG_HPP