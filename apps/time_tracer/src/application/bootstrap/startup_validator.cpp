// bootstrap/startup_validator.cpp
#include "application/bootstrap/startup_validator.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

// [修改] 直接引入 PluginValidator
#include "infrastructure/config/validator/plugins/facade/plugin_validator.hpp"
#include "shared/types/ansi_colors.hpp"

namespace fs = std::filesystem;

auto StartupValidator::ValidateEnvironment(const AppConfig& config) -> bool {
  // 1. 定义需要检查的插件列表
  // (原本这些列表是在 ConfigFacade
  // 中定义的，现在移到这里，因为这是启动环境检查的一部分)
  const std::vector<std::string> kExpectedPlugins = {
      "DayMdFormatter",   "DayTexFormatter",   "DayTypFormatter",
      "MonthMdFormatter", "MonthTexFormatter", "MonthTypFormatter",
      "RangeMdFormatter", "RangeTexFormatter", "RangeTypFormatter"};

  // 2. 准备路径
  fs::path plugins_dir = fs::path(config.exe_dir_path) / "plugins";
  // 核心库通常在 bin 目录（exe 同级）
  fs::path bin_dir = config.exe_dir_path;

  // 3. 执行验证
  // 3.1 验证格式化器插件
  bool plugins_ok = PluginValidator::Validate(plugins_dir, kExpectedPlugins);

  // 3.2 验证核心共享库 (reports_shared.dll)
  // 这是一个关键依赖，必须存在
  bool core_ok = PluginValidator::Validate(bin_dir, {"reports_shared"});

  if (!plugins_ok || !core_ok) {
    namespace colors = time_tracer::common::colors;
    std::cerr << colors::kRed
              << "Fatal: Runtime environment validation failed (Missing DLLs)."
              << colors::kReset << std::endl;
    return false;
  }

  return true;
}
