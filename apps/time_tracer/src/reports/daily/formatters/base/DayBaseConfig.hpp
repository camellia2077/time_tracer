// reports/daily/formatters/base/DayBaseConfig.hpp
#ifndef DAY_BASE_CONFIG_HPP
#define DAY_BASE_CONFIG_HPP

#include "reports/shared/shared_api.hpp"
#include <string>
#include <map>
#include <vector> 
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"
#include <filesystem>

// [修改] 升级结构体以支持递归和数据库映射
struct StatisticItemConfig {
    std::string label;
    std::string db_column; // [新增] 对应的数据库列名
    bool show = true;
    std::vector<StatisticItemConfig> sub_items; // [新增] 支持无限层级嵌套
};

DISABLE_C4251_WARNING

/**
 * @class DayBaseConfig
 * @brief 日报配置的基类，封装了所有日报格式共享的配置项。
 */
class REPORTS_SHARED_API DayBaseConfig {
public:
    explicit DayBaseConfig(const std::filesystem::path& config_path);
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

    // [新增] 通用的项目统计标题 getter
    const std::string& get_project_breakdown_label() const;
    
    // [修改] 返回类型改为 vector 以保持 JSON 数组的顺序
    const std::vector<StatisticItemConfig>& get_statistics_items() const;

protected:
    nlohmann::json config_json_; 

private:
    void load_base_config();

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

    // 通用的项目统计标题变量
    std::string project_breakdown_label_;
    
    // 存储结构改为 vector
    std::vector<StatisticItemConfig> statistics_items_;
};

ENABLE_C4251_WARNING

#endif // DAY_BASE_CONFIG_HPP