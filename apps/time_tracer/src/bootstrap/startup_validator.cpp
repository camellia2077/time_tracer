// bootstrap/startup_validator.cpp
#include "startup_validator.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

// [修改] 直接引入 PluginValidator
#include "common/ansi_colors.hpp"
#include "config/validator/plugins/facade/plugin_validator.hpp"

namespace fs = std::filesystem;

auto StartupValidator::validate_environment(const AppConfig& config) -> bool {
  // 1. 定义需要检查的插件列表
  // (原本这些列表是在 ConfigFacade
  // 中定义的，现在移到这里，因为这是启动环境检查的一部分)
  const std::vector<std::string> kExpectedPlugins = {
      "DayMdFormatter",    "DayTexFormatter",    "DayTypFormatter",
      "MonthMdFormatter",  "MonthTexFormatter",  "MonthTypFormatter",
      "PeriodMdFormatter", "PeriodTexFormatter", "PeriodTypFormatter"};

  // 2. 准备路径
  fs::path plugins_dir = fs::path(config.exe_dir_path) / "plugins";
  // 核心库通常在 bin 目录（exe 同级）
  fs::path bin_dir = config.exe_dir_path;

  // 3. 执行验证
  PluginValidator validator;

  // 3.1 验证格式化器插件
  bool plugins_ok = validator.validate(plugins_dir, kExpectedPlugins);

  // 3.2 验证核心共享库 (reports_shared.dll)
  // 这是一个关键依赖，必须存在
  bool core_ok = validator.validate(bin_dir, {"reports_shared"});

  if (!plugins_ok || !core_ok) {
    std::cerr << RED_COLOR
              << "Fatal: Runtime environment validation failed (Missing DLLs)."
              << RESET_COLOR << std::endl;
    return false;
  }

  return true;
}