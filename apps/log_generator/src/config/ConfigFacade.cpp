// config/ConfigFacade.cpp
#include "ConfigFacade.hpp"
#include <set>
#include <iostream>

namespace ConfigFacade {
    bool validate(const TomlConfigData& config_data, std::vector<std::string>& errors) {
        errors.clear();
        if (config_data.mapped_activities.empty()) {
            errors.push_back("配置错误: 未能从 mapping_config.toml 中加载到任何活动映射键。");
        }

        return errors.empty();
    }
}