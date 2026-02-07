// infrastructure/config/loader/toml_loader_utils.hpp
#ifndef CONFIG_LOADER_TOML_LOADER_UTILS_H_
#define CONFIG_LOADER_TOML_LOADER_UTILS_H_

#include <toml++/toml.h>

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"

namespace fs = std::filesystem;

namespace TomlLoaderUtils {

// --- IO ---
/**
 * @brief 读取并解析 TOML 文件
 * @throws std::runtime_error 如果文件不存在或解析失败
 */
auto ReadToml(const fs::path& path) -> toml::table;

// --- 模板辅助函数 ---

// 获取可选值
template <typename T>
auto GetOptional(const toml::node_view<const toml::node>& node,
                 const T& default_value) -> T {
  return node.value_or(default_value);
}

// 获取必须值
template <typename T>
auto GetRequired(const toml::table& tbl, const std::string& key) -> T {
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

// 解析统计项(支持递归)
void ParseStatisticsItems(const toml::array* arr,
                          std::vector<ReportStatisticsItem>& out_items);

// 解析样式 (Tex / Typst)
void FillTexStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout);
void FillTypStyle(const toml::table& tbl, FontConfig& fonts,
                  LayoutConfig& layout);

// 解析标签 (Daily / Range)
void FillDailyLabels(const toml::table& tbl, DailyReportLabels& labels);
void FillRangeLabels(const toml::table& tbl, RangeReportLabels& labels);
void FillMonthlyLabels(const toml::table& tbl, MonthlyReportLabels& labels);
void FillPeriodLabels(const toml::table& tbl, PeriodReportLabels& labels);
void FillWeeklyLabels(const toml::table& tbl, WeeklyReportLabels& labels);
void FillYearlyLabels(const toml::table& tbl, YearlyReportLabels& labels);

// 解析通用部分
void FillKeywordColors(const toml::table& tbl,
                       std::map<std::string, std::string>& colors);
}  // namespace TomlLoaderUtils

#endif  // CONFIG_LOADER_TOML_LOADER_UTILS_H_
