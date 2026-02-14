// infrastructure/config/loader/report_config_loader.hpp
#ifndef INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_
#define INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_

#include <filesystem>
#include <string>
#include <vector>

#include "infrastructure/config/models/report_config_models.hpp"  // 假设你的模型定义在这里

/**
 * @class ReportConfigLoader
 * @brief 负责读取配置文件并将其解析为 C++ 结构体。
 * 实现了配置格式(JSON)与业务模型(Structs)的解耦。
 */
class ReportConfigLoader {
 public:
  // --- 日报加载器 ---
  static auto LoadDailyTexConfig(const std::filesystem::path& path)
      -> DailyTexConfig;
  static auto LoadDailyTypConfig(const std::filesystem::path& path)
      -> DailyTypConfig;
  static auto LoadDailyMdConfig(const std::filesystem::path& path)
      -> DailyMdConfig;

  // --- 月报加载器 ---
  static auto LoadMonthlyTexConfig(const std::filesystem::path& path)
      -> MonthlyTexConfig;
  static auto LoadMonthlyTypConfig(const std::filesystem::path& path)
      -> MonthlyTypConfig;
  static auto LoadMonthlyMdConfig(const std::filesystem::path& path)
      -> MonthlyMdConfig;

  // --- 周期报告加载器 ---
  static auto LoadPeriodTexConfig(const std::filesystem::path& path)
      -> PeriodTexConfig;
  static auto LoadPeriodTypConfig(const std::filesystem::path& path)
      -> PeriodTypConfig;
  static auto LoadPeriodMdConfig(const std::filesystem::path& path)
      -> PeriodMdConfig;

  // --- Weekly Loaders ---
  static auto LoadWeeklyTexConfig(const std::filesystem::path& path)
      -> WeeklyTexConfig;
  static auto LoadWeeklyTypConfig(const std::filesystem::path& path)
      -> WeeklyTypConfig;
  static auto LoadWeeklyMdConfig(const std::filesystem::path& path)
      -> WeeklyMdConfig;

  // --- Yearly Loaders ---
  static auto LoadYearlyTexConfig(const std::filesystem::path& path)
      -> YearlyTexConfig;
  static auto LoadYearlyTypConfig(const std::filesystem::path& path)
      -> YearlyTypConfig;
  static auto LoadYearlyMdConfig(const std::filesystem::path& path)
      -> YearlyMdConfig;
};

#endif  // INFRASTRUCTURE_CONFIG_LOADER_REPORT_CONFIG_LOADER_H_
