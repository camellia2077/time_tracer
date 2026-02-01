// common/app_context.hpp
#ifndef COMMON_APP_CONTEXT_H_
#define COMMON_APP_CONTEXT_H_

#include <optional>
#include <string>
#include <vector>

#include "common/config_types.hpp"

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

}  // namespace Core
#endif  // COMMON_APP_CONTEXT_H_