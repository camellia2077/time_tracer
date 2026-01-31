// cli/framework/core/command_parser.cpp
#include "cli/framework/core/command_parser.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>

// [修改] 初始化配置
CommandParser::CommandParser(const std::vector<std::string>& args,
                             const ParserConfig& config)
    : raw_args_(args), config_(config) {
  if (args.size() < 2) {
    throw std::runtime_error("No command provided.");
  }
  command_ = raw_args_[1];
  parse();
}

auto CommandParser::get_command() const -> std::string {
  return command_;
}

auto CommandParser::get_filtered_args() const
    -> const std::vector<std::string>& {
  return filtered_args_;
}

auto CommandParser::get_raw_arg(size_t index) const -> std::string {
  if (index >= raw_args_.size()) {
    throw std::out_of_range("Argument index out of range.");
  }
  return raw_args_[index];
}

auto CommandParser::get_option(const std::vector<std::string>& keys) const
    -> std::optional<std::string> {
  for (const auto& key : keys) {
    auto it = std::ranges::find(raw_args_, key);
    if (it != raw_args_.end() && std::next(it) != raw_args_.end()) {
      return *std::next(it);
    }
  }
  return std::nullopt;
}

auto CommandParser::has_flag(const std::vector<std::string>& keys) const
    -> bool {
  for (const auto& key : keys) {
    if (std::ranges::find(raw_args_, key) != raw_args_.end()) {
      return true;
    }
  }
  return false;
}

void CommandParser::parse() {
  filtered_args_ = filter_global_options(raw_args_);
}

// [修改] 使用 config_ 中的定义进行过滤，彻底移除硬编码
auto CommandParser::filter_global_options(
    const std::vector<std::string>& original_args) -> std::vector<std::string> {
  std::vector<std::string> filtered;

  for (size_t i = 0; i < original_args.size(); ++i) {
    const auto& arg = original_args[i];

    // 1. 检查是否是带值选项 (Value Options)
    if (std::ranges::find(config_.global_value_options,

                          arg) != config_.global_value_options.end()) {
      i++;  // 跳过 key 和 value
      continue;
    }

    // 2. 检查是否是布尔选项 (Flags)
    if (std::ranges::find(config_.global_flag_options,

                          arg) != config_.global_flag_options.end()) {
      continue;  // 仅跳过 key
    }

    filtered.push_back(arg);
  }
  return filtered;
}
