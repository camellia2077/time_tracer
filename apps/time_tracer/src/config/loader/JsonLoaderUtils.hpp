// config/loader/JsonLoaderUtils.hpp
#ifndef JSON_LOADER_UTILS_HPP
#define JSON_LOADER_UTILS_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>
#include "common/config/ReportConfigModels.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace JsonLoaderUtils {

    // --- IO ---
    json read_json(const fs::path& path);

    // --- 模板辅助函数 (必须在头文件中实现) ---
    template <typename T>
    T get_optional(const json& j, const std::string& key, const T& default_value) {
        if (j.contains(key) && !j[key].is_null()) {
            return j.at(key).get<T>();
        }
        return default_value;
    }

    template <typename T>
    T get_required(const json& j, const std::string& key) {
        if (!j.contains(key)) {
            throw std::runtime_error("Missing required config key: " + key);
        }
        return j.at(key).get<T>();
    }

    // --- 逻辑填充辅助函数 ---
    
    // 解析统计项
    void parse_statistics_items(const json& j_items, std::vector<ReportStatisticsItem>& out_items);
    
    // 解析样式 (Tex / Typst)
    void fill_tex_style(const json& j, FontConfig& fonts, LayoutConfig& layout);
    void fill_typ_style(const json& j, FontConfig& fonts, LayoutConfig& layout);
    
    // 解析标签 (Daily / Monthly / Period)
    void fill_daily_labels(const json& j, DailyReportLabels& labels);
    void fill_monthly_labels(const json& j, MonthlyReportLabels& labels);
    void fill_period_labels(const json& j, PeriodReportLabels& labels);

    // 解析通用部分
    void fill_keyword_colors(const json& j, std::map<std::string, std::string>& colors);
}

#endif // JSON_LOADER_UTILS_HPP