// config/loader/report_config_loader.hpp
#ifndef CONFIG_LOADER_REPORT_CONFIG_LOADER_H_
#define CONFIG_LOADER_REPORT_CONFIG_LOADER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "common/config/report_config_models.hpp"  // 假设你的模型定义在这里

/**
 * @class ReportConfigLoader
 * @brief 负责读取配置文件并将其解析为 C++ 结构体。
 * 实现了配置格式(JSON)与业务模型(Structs)的解耦。
 */
class ReportConfigLoader {
 public:
  // --- 日报加载器 ---
  static DailyTexConfig loadDailyTexConfig(const std::filesystem::path& path);
  static DailyTypConfig loadDailyTypConfig(const std::filesystem::path& path);
  static DailyMdConfig loadDailyMdConfig(const std::filesystem::path& path);

  // --- 月报加载器 ---
  static MonthlyTexConfig loadMonthlyTexConfig(
      const std::filesystem::path& path);
  static MonthlyTypConfig loadMonthlyTypConfig(
      const std::filesystem::path& path);
  static MonthlyMdConfig loadMonthlyMdConfig(const std::filesystem::path& path);

  // --- 周期报告加载器 ---
  static PeriodTexConfig loadPeriodTexConfig(const std::filesystem::path& path);
  static PeriodTypConfig loadPeriodTypConfig(const std::filesystem::path& path);
  static PeriodMdConfig loadPeriodMdConfig(const std::filesystem::path& path);
};

#endif  // CONFIG_LOADER_REPORT_CONFIG_LOADER_H_