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
// [修改] 移除 help_formatter.hpp 的直接引用，因为不再直接调用 print_full_usage
// #include "cli/impl/utils/help_formatter.hpp"


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

  // [新增] 如果没有参数，自动追加 --help 以便进入 CliApplication 的帮助流程
  if (args.size() < 2) {
      args.push_back("--help");
  }

  // 此时 args.size() 肯定 >= 2
  // 检查是否是帮助模式 (跳过部分验证)
  bool is_help_mode = (args[1] == "-h" || args[1] == "--help");

  // 彩蛋逻辑
  if (args[1] == "tracer") {
    std::println("\n{}{}{}\n", CYAN_COLOR,
                 "  \"Cheers, love! The timetracer is here.\"", RESET_COLOR);
    return 0;
  }
  if (args[1] == "motto" || args[1] == "zen") {
    std::println("");
    std::println("{}  \"Trace your time, log your life.\"{}\n", CYAN_COLOR,
                 RESET_COLOR);
    return 0;
  }

  // [修改] 移除了直接调用 print_full_usage 的代码，改为委托给 CliApplication
  // 原来的 if (args.size() < 2) 检查已在上方处理

  // 处理缩写命令
  if (!is_help_mode) {
      if (args[1] == "pre") args[1] = "preprocess";
      // 输入 "blink" 时，视为 "ingest"
      if (args[1] == "blink") args[1] = "ingest";
  }

  const std::string& command = args[1];

  // 处理全局 version 参数 (help 参数已移交 CliApplication)
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
    // [修改] 如果是帮助模式，跳过环境验证，避免因配置错误导致无法查看帮助
    if (!is_help_mode) {
        if (!StartupValidator::validate_environment(config)) {
          std::println(std::cerr,
                       "\n{}Configuration validation failed. Please check the "
                       "errors above.{}\n",
                       RED_COLOR, RESET_COLOR);
          return 1;  // 验证失败，直接退出
        }
    }

    // 3. 执行业务
    // 验证通过，启动 CLI 控制器处理具体命令
    // [说明] CliApplication 内部会检测 --help 标记并使用正确的上下文生成动态帮助
    CliApplication controller(args);
    controller.execute();

  } catch (const std::exception& e) {
    // 统一错误处理
    std::println(std::cerr, "{}Startup Error: {}{}", RED_COLOR, e.what(),
                 RESET_COLOR);

    if (std::string(e.what()).find("command") != std::string::npos ||
        std::string(e.what()).find("argument") != std::string::npos) {
      std::println("\nUse '{} --help' for more information.", args[0]);
    }
    return 1;
  }

  return 0;
}