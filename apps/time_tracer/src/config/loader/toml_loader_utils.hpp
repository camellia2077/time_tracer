// config/loader/toml_loader_utils.hpp
#ifndef CONFIG_LOADER_TOML_LOADER_UTILS_HPP_
#define CONFIG_LOADER_TOML_LOADER_UTILS_HPP_

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <toml++/toml.h>
#include "common/config/report_config_models.hpp"

namespace fs = std::filesystem;

namespace TomlLoaderUtils {

    // --- IO ---
    /**
     * @brief 读取并解析 TOML 文件
     * @throws std::runtime_error 如果文件不存在或解析失败
     */
    toml::table read_toml(const fs::path& path);

    // --- 模板辅助函数 ---
    
    // 获取可选值
    template <typename T>
    T get_optional(const toml::node_view<const toml::node>& node, const T& default_value) {
        return node.value_or(default_value);
    }

    // 获取必需值
    template <typename T>
    T get_required(const toml::table& tbl, const std::string& key) {
        if (!tbl.contains(key)) {
            throw std::runtime_error("Missing required config key: " + key);
        }
        auto val = tbl[key].value<T>();
        if (!val) {
            throw std::runtime_error("Invalid type for key: " + key);
        }
        return *val;
    }

    // --- 逻辑填充辅助函数 ---
    
    // 解析统计项 (支持递归)
    void parse_statistics_items(const toml::array* arr, std::vector<ReportStatisticsItem>& out_items);
    
    // 解析样式 (Tex / Typst)
    void fill_tex_style(const toml::table& tbl, FontConfig& fonts, LayoutConfig& layout);
    void fill_typ_style(const toml::table& tbl, FontConfig& fonts, LayoutConfig& layout);
    
    // 解析标签 (Daily / Monthly / Period)
    void fill_daily_labels(const toml::table& tbl, DailyReportLabels& labels);
    void fill_monthly_labels(const toml::table& tbl, MonthlyReportLabels& labels);
    void fill_period_labels(const toml::table& tbl, PeriodReportLabels& labels);

    // 解析通用部分
    void fill_keyword_colors(const toml::table& tbl, std::map<std::string, std::string>& colors);
}

#endif // CONFIG_LOADER_TOML_LOADER_UTILS_HPP_