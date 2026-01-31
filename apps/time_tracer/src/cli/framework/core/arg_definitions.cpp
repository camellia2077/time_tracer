// cli/framework/core/arg_definitions.cpp
#include "cli/framework/core/arg_definitions.hpp"

#include <stdexcept>

auto ParsedArgs::get(const std::string& name) const -> std::string {
  auto it = values_.find(name);
  if (it != values_.end()) {
    return it->second;
  }
  return "";  // 如果没找到，返回空字符串（或者根据需求抛出异常）
}

auto ParsedArgs::has(const std::string& name) const -> bool {
  return values_.contains(name);
}

auto ParsedArgs::get_as_int(const std::string& name) const -> int {
  auto it = values_.find(name);
  if (it != values_.end()) {
    try {
      return std::stoi(it->second);
    } catch (...) {
      throw std::runtime_error("Argument '" + name +
                               "' is not a valid integer: " + it->second);
    }
  }
  throw std::runtime_error("Argument not found: " + name);
}
