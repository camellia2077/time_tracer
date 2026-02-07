// api/cli/framework/core/arg_definitions.cpp
#include "api/cli/framework/core/arg_definitions.hpp"

#include <stdexcept>

auto ParsedArgs::Get(const std::string& name) const -> std::string {
  auto iterator = values_.find(name);
  if (iterator != values_.end()) {
    return iterator->second;
  }
  return "";  // 如果没找到，返回空字符串（或者根据需求抛出异常）
}

auto ParsedArgs::Has(const std::string& name) const -> bool {
  return values_.contains(name);
}

auto ParsedArgs::GetAsInt(const std::string& name) const -> int {
  auto iterator = values_.find(name);
  if (iterator != values_.end()) {
    try {
      return std::stoi(iterator->second);
    } catch (...) {
      throw std::runtime_error("Argument '" + name +
                               "' is not a valid integer: " + iterator->second);
    }
  }
  throw std::runtime_error("Argument not found: " + name);
}
