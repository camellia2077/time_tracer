// config/ConfigValidator.cpp
#include "ConfigValidator.h"
#include <set>
#include <iostream>

namespace ConfigValidator {
    bool validate(const JsonConfigData& config_data, std::vector<std::string>& errors) {
        errors.clear();

        // 使用 std::set 来检查 common_activities 中的重复项
        std::set<std::string> unique_activities;
        for (const auto& activity : config_data.activities) {
            // .second 会在插入新元素时返回true，如果元素已存在则返回false
            if (!unique_activities.insert(activity).second) {
                errors.push_back("配置错误: 在 common_activities.json 中发现重复的活动项 -> " + activity);
            }
        }

        return errors.empty();
    }
}