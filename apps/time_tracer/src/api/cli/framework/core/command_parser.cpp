// api/cli/framework/core/command_parser.cpp
#include "api/cli/framework/core/command_parser.hpp"

#include <algorithm>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>

#include "api/cli/framework/core/arg_definitions.hpp"

namespace {
constexpr size_t kMinArgsForCommand = 2;
constexpr size_t kCommandArgIndex = 1;
}  // namespace

// [修改] 初始化配置
CommandParser::CommandParser(std::vector<std::string> args, ParserConfig config)
    : raw_args_(std::move(args)), config_(std::move(config)) {
  if (raw_args_.size() < kMinArgsForCommand) {
    throw std::runtime_error("No command provided.");
  }
  command_ = raw_args_[kCommandArgIndex];
  Parse();
}

auto CommandParser::GetCommand() const -> std::string {
  return command_;
}

auto CommandParser::GetFilteredArgs() const -> const std::vector<std::string>& {
  return filtered_args_;
}

auto CommandParser::GetPositionalArgs() const -> std::vector<std::string> {
  std::vector<std::string> positionals;
  for (const auto& arg : filtered_args_) {
    if (!arg.empty() && arg[0] != '-') {
      positionals.push_back(arg);
    }
  }
  return positionals;
}

auto CommandParser::GetPositionalArgs(std::span<const ArgDef> definitions) const
    -> std::vector<std::string> {
  // 收集所有 Option 类型的 keys
  std::vector<std::string> option_keys;
  for (const auto& def : definitions) {
    if (def.type == ArgType::kOption) {
      for (const auto& key : def.keys) {
        option_keys.push_back(key);
      }
    }
  }

  std::vector<std::string> positionals;
  for (size_t i = 0; i < filtered_args_.size(); ++i) {
    const auto& arg = filtered_args_[i];

    // 检查是否是 Option key
    bool is_option_key =
        std::ranges::find(option_keys, arg) != option_keys.end();
    if (is_option_key) {
      // 跳过选项本身和它的值
      ++i;
      continue;
    }

    // 跳过以 '-' 开头的参数 (flags)
    if (!arg.empty() && arg[0] == '-') {
      continue;
    }

    positionals.push_back(arg);
  }
  return positionals;
}

auto CommandParser::GetRawArg(size_t index) const -> std::string {
  if (index >= raw_args_.size()) {
    throw std::out_of_range("Argument index out of range.");
  }
  return raw_args_[index];
}

auto CommandParser::GetOption(const std::vector<std::string>& keys) const
    -> std::optional<std::string> {
  for (const auto& key : keys) {
    auto found_it = std::ranges::find(raw_args_, key);
    if (found_it != raw_args_.end() && std::next(found_it) != raw_args_.end()) {
      return *std::next(found_it);
    }
  }
  return std::nullopt;
}

auto CommandParser::HasFlag(const std::vector<std::string>& keys) const
    -> bool {
  return std::ranges::any_of(keys, [this](const std::string& key) -> bool {
    return std::ranges::find(raw_args_, key) != raw_args_.end();
  });
}

void CommandParser::Parse() {
  filtered_args_ = FilterGlobalOptions(raw_args_);
}

auto CommandParser::FilterGlobalOptions(
    const std::vector<std::string>& original_args) -> std::vector<std::string> {
  std::vector<std::string> filtered;

  for (size_t i = 0; i < original_args.size(); ++i) {
    const auto& arg = original_args[i];

    if (std::ranges::find(config_.global_value_options, arg) !=
        config_.global_value_options.end()) {
      i++;
      continue;
    }

    if (std::ranges::find(config_.global_flag_options, arg) !=
        config_.global_flag_options.end()) {
      continue;
    }

    filtered.push_back(arg);
  }
  return filtered;
}
