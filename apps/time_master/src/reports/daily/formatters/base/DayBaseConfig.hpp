// reports/daily/formatters/base/DayBaseConfig.hpp
#ifndef DAY_BASE_CONFIG_HPP
#define DAY_BASE_CONFIG_HPP

#include "reports/shared/shared_api.hpp" // <-- [新增] 1. 包含API宏
#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"

// 用于存储单个统计项配置的结构体
// (这个结构体保持不变，因为它不是一个被导出的类)
struct StatisticItemConfig {
    std::string label;
    bool show = true;
};

DISABLE_C4251_WARNING // <-- [新增] 2. 禁用C4251警告 (因为有std::string/map)

/**
 * @class DayBaseConfig
 * @brief 日报配置的基类，封装了所有日报格式共享的配置项。
 * ... (注释保持不变) ...
 */
class REPORTS_SHARED_API DayBaseConfig { // <-- [修改] 3. 添加API宏
public:
    explicit DayBaseConfig(const std::string& config_path);
    virtual ~DayBaseConfig() = default;

    // --- 通用配置项的 Getters ---
    const std::string& get_title_prefix() const;
    const std::string& get_date_label() const;
    const std::string& get_total_time_label() const;
    const std::string& get_status_label() const;
    const std::string& get_sleep_label() const;
    const std::string& get_getup_time_label() const;
    const std::string& get_remark_label() const;
    const std::string& get_exercise_label() const;
    const std::string& get_no_records() const;
    const std::string& get_statistics_label() const;
    const std::string& get_all_activities_label() const;
    const std::string& get_activity_remark_label() const;
    const std::string& get_activity_connector() const;
    const std::map<std::string, StatisticItemConfig>& get_statistics_items() const;

protected:
    // 子类可以访问解析后的JSON对象来加载自己的特有配置
    nlohmann::json config_json_; 

private:
    void load_base_config(); // 私有方法，用于加载所有通用配置

    // --- 所有共享的成员变量 ---
    std::string title_prefix_;
    std::string date_label_;
    std::string total_time_label_;
    std::string status_label_;
    std::string sleep_label_;
    std::string getup_time_label_;
    std::string remark_label_;
    std::string exercise_label_;
    std::string no_records_;
    std::string statistics_label_;
    std::string all_activities_label_;
    std::string activity_remark_label_;
    std::string activity_connector_; 
    std::map<std::string, StatisticItemConfig> statistics_items_;
};

ENABLE_C4251_WARNING // <-- [新增] 4. 恢复C4251警告

#endif // DAY_BASE_CONFIG_HPP