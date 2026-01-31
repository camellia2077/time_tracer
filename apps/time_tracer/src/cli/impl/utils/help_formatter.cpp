// cli/impl/utils/help_formatter.cpp
#include "help_formatter.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cli/framework/core/arg_definitions.hpp"
#include "cli/framework/interfaces/i_command.hpp"
#include "common/ansi_colors.hpp"

void print_full_usage(
    const char* app_name,
    const std::vector<std::pair<std::string, std::unique_ptr<ICommand>>>&
        commands) {
  std::cout << GREEN_COLOR << "TimeTracer" << RESET_COLOR
            << ": A command-line tool for time data processing.\n\n";
  std::cout << "Usage: " << app_name
            << " <command> [arguments...] [options...]\n\n";

  std::cout << "Available Commands:\n";

  for (const auto& [name, cmd] : commands) {
    if (!cmd) {
      continue;
    }

    auto defs = cmd->get_definitions();

    // 1. 构建 Usage 字符串
    std::stringstream usage_ss;
    usage_ss << "  " << GREEN_COLOR << std::left << std::setw(15) << name
             << RESET_COLOR;

    // 分离位置参数和选项
    std::vector<ArgDef> positionals;
    std::vector<ArgDef> options;

    for (const auto& def : defs) {
      if (def.type == ArgType::Positional) {
        positionals.push_back(def);
      } else {
        options.push_back(def);
      }
    }

    // 按索引排序位置参数
    std::ranges::sort(positionals,
                      [](const ArgDef& a, const ArgDef& b) -> bool {
                        return a.position_index < b.position_index;
                      });

    // 打印简短描述 (如果有)
    // 注意：建议在 ICommand 子类中覆盖 get_help() 返回单行描述
    std::string description = cmd->get_help();
    if (description == "Auto generated help...") {
      description = "";
    }
    std::cout << usage_ss.str() << " " << description << "\n";

    // 2. 打印详细参数格式
    std::cout << "    Usage: " << name;
    for (const auto& pos : positionals) {
      std::cout << " <" << pos.name << ">";
    }
    if (!options.empty()) {
      std::cout << " [options]";
    }
    std::cout << "\n";

    // 3. 打印详细参数列表
    if (!positionals.empty()) {
      for (const auto& def : positionals) {
        std::cout << "      " << std::left << std::setw(20)
                  << ("<" + def.name + ">") << ": " << def.help
                  << (def.required ? " (Required)" : "") << "\n";
      }
    }

    if (!options.empty()) {
      for (const auto& def : options) {
        std::stringstream keys_ss;
        for (size_t i = 0; i < def.keys.size(); ++i) {
          keys_ss << def.keys[i] << (i < def.keys.size() - 1 ? ", " : "");
        }
        std::cout << "      " << std::left << std::setw(20) << keys_ss.str()
                  << ": " << def.help << "\n";
      }
    }
    std::cout << "\n";
  }

  std::cout << "Global Options:\n";
  std::cout << "  --help, -h          Show this help message.\n";
  std::cout << "  --version, -v       Show program version.\n";
}
