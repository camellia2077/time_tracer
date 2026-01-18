// main_cli.cpp
#include <filesystem>
#include <iostream>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

// --- Windows-specific include for console functions ---
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- 核心模块 ---
#include "bootstrap/startup_validator.hpp"  // 引入验证器

#include "common/config/app_config.hpp"
#include "config/config_loader.hpp"         

#include "cli/impl/app/cli_application.hpp"
// --- 工具与信息 ---
#include "cli/framework/interfaces/i_command.hpp"
#include "cli/impl/utils/help_formatter.hpp"


#include "common/ansi_colors.hpp"
#include "common/version.hpp"

int main(int argc, char* argv[]) {
// --- Console Setup (Windows Only) ---
#if defined(_WIN32) || defined(_WIN64)
  SetConsoleOutputCP(CP_UTF8);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode)) {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }
  }
#endif

  std::vector<std::string> args(argv, argv + argc);

  // 彩蛋逻辑
  if (args.size() > 1 && args[1] == "tracer") {
    std::println("\n{}{}{}\n", CYAN_COLOR,
                 "  \"Cheers, love! The timetracer is here.\"", RESET_COLOR);
    return 0;
  }
  if (args.size() > 1 && (args[1] == "motto" || args[1] == "zen")) {
    std::println("");
    std::println("{}  \"Trace your time, log your life.\"{}\n", CYAN_COLOR,
                 RESET_COLOR);
    return 0;
  }

  // 如果没有参数，打印帮助
  if (args.size() < 2) {
    print_full_usage(args[0].c_str());
    return 1;
  }

  // 处理缩写命令
  if (args[1] == "pre") args[1] = "preprocess";
  // 输入 "blink" 时，视为 "ingest"
  if (args[1] == "blink") args[1] = "ingest";

  const std::string& command = args[1];

  // 处理全局 help/version 参数
  if (command == "-h" || command == "--help") {
    print_full_usage(args[0].c_str());
    return 0;
  }
  if (command == "-v" || command == "--version") {
    std::println("TimeMaster Command Version: {}", AppInfo::VERSION);
    std::println("Last Updated:  {}", AppInfo::LAST_UPDATED);
    return 0;
  }

  // --- 启动流程：加载配置 -> 验证环境 -> 执行业务 ---
  try {
    // 1. 加载配置
    ConfigLoader boot_loader(args[0]);
    AppConfig config = boot_loader.load_configuration();

    // 2. 验证环境 (委托给 StartupValidator)
    if (!StartupValidator::validate_environment(config)) {
      std::println(std::cerr,
                   "\n{}Configuration validation failed. Please check the "
                   "errors above.{}\n",
                   RED_COLOR, RESET_COLOR);
      return 1;  // 验证失败，直接退出
    }

    // 3. 执行业务
    // 验证通过，启动 CLI 控制器处理具体命令
    CliApplication controller(args);
    controller.execute();

  } catch (const std::exception& e) {
    // 统一错误处理
    std::println(std::cerr, "{}Startup Error: {}{}", RED_COLOR, e.what(),
                 RESET_COLOR);

    if (std::string(e.what()).find("command") != std::string::npos ||
        std::string(e.what()).find("argument") != std::string::npos) {
      std::println("\nUse '{}' for more information.", args[0]);
    }
    return 1;
  }

  return 0;
}