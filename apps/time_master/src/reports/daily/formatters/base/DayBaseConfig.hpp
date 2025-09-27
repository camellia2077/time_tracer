// reports/daily/formatters/base/DayBaseConfig.hpp
#ifndef DAY_BASE_CONFIG_HPP
#define DAY_BASE_CONFIG_HPP

#include <string>
#include <map>
#include <nlohmann/json.hpp>
#include "reports/shared/utils/config/ConfigUtils.hpp"

// 用于存储单个统计项配置的结构体
struct StatisticItemConfig {
    std::string label;
    bool show = true;
};

/**
 * @class DayBaseConfig
 * @brief 日报配置的基类，封装了所有日报格式共享的配置项。
 *
 * 这个类负责从 JSON 配置文件中加载所有通用的标签、消息和设置。
 * 子类（如 DayMdConfig, DayTexConfig）继承自此类，并只加载它们特有的配置。
 *
 * @note 
 * 对应的 JSON 配置文件必须包含以下通用键：
 * - "date_label": (string) "日期"标签
 * - "total_time_label": (string) "总时长"标签
 * - "status_label": (string) "状态"标签
 * - "sleep_label": (string) "睡眠"标签
 * - "getup_time_label": (string) "起床时间"标签
 * - "remark_label": (string) "备注"标签
 * - "exercise_label": (string) "锻炼"标签
 * - "no_records" / "no_records_message": (string) 无记录时的提示信息
 * - "statistics_label": (string) "统计数据"部分的标题
 * - "all_activities_label": (string) "所有活动"部分的标题
 * - "activity_remark_label": (string) 单个活动备注的标签
 * - "activity_connector": (string) 活动路径中用于连接的字符串
 * - "statistics_items": (object) 一个包含多个统计项配置的对象
 *
 * @note 
 * 键 "title_prefix" (string) 是可选的，主要用于 Markdown 和 Typst 报告。
 */
class DayBaseConfig {
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

#endif // DAY_BASE_CONFIG_HPP