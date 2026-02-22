// infrastructure/config/config_facade.cpp
#include "infrastructure/config/config_facade.hpp"

#include <iostream>
#include <set>

namespace ConfigFacade {
auto validate(const TomlConfigData& config_data,
              std::vector<std::string>& errors) -> bool {
  errors.clear();
  if (config_data.mapped_activities.empty()) {
    errors.emplace_back(
        "配置错误: 未能从 alias_mapping.toml 中加载到任何活动映射键。");
  }

  return errors.empty();
}
}  // namespace ConfigFacade
