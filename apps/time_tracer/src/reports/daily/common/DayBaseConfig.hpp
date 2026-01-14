// reports/daily/common/DayBaseConfig.hpp
#ifndef DAY_BASE_CONFIG_HPP
#define DAY_BASE_CONFIG_HPP

#include "reports/shared/api/shared_api.hpp"
#include <string>
#include <map>
#include <vector> 
#include <nlohmann/json.hpp>
// [修改] 不再依赖 ConfigUtils 进行 IO 操作
// #include "reports/shared/utils/config/ConfigUtils.hpp"

// [修改] 升级结构体以支持递归和数据库映射
struct StatisticItemConfig {
    std::string label;
    std::string db_column;
    bool show = true;
    std::vector<StatisticItemConfig> sub_items;
};

DISABLE_C4251_WARNING

/**
 * @class DayBaseConfig
 * @brief 日报配置的基类，封装了所有日报格式共享的配置项。
 * * [架构变更] 此类不再负责文件读取。配置数据由外部模块读取后注入。
 */
class REPORTS_SHARED_API DayBaseConfig {
public:
    // [修改] 构造函数接收 JSON 对象
    explicit DayBaseConfig(const nlohmann::json& config);
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

    const std::string& get_project_breakdown_label() const;
    
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

    std::string project_breakdown_label_;
    
    std::vector<StatisticItemConfig> statistics_items_;
};

ENABLE_C4251_WARNING

#endif // DAY_BASE_CONFIG_HPP