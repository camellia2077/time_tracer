// common/AppContext.h
#ifndef COMMON_APPCONTEXT_H
#define COMMON_APPCONTEXT_H

#include "common/ConfigTypes.hpp" // [修改] 不再依赖 config/Config.h
#include <vector>
#include <string>
#include <optional>

namespace Core {

    /**
     * @brief 应用程序上下文，包含运行所需的全部配置数据。
     * 由 ConfigHandler 生成，供 WorkflowHandler 使用。
     */
    struct AppContext {
        Config config;
        std::vector<std::string> all_activities;
        std::optional<DailyRemarkConfig> remarks;
        std::optional<ActivityRemarkConfig> activity_remarks;
        std::vector<std::string> wake_keywords;
    };

}
#endif // COMMON_APPCONTEXT_H