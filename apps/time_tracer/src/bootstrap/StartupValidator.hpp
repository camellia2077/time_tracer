// bootstrap/StartupValidator.hpp
#ifndef STARTUP_VALIDATOR_HPP
#define STARTUP_VALIDATOR_HPP

// [修复] 更新包含路径
#include "common/config/AppConfig.hpp"

/**
 * @class StartupValidator
 * @brief 负责在应用程序启动前验证运行环境的完整性。
 * * 包括检查插件是否存在、配置文件是否可读以及配置内容是否符合规范。
 */
class StartupValidator {
public:
    /**
     * @brief 执行完整的环境验证流程。
     * * @param config 应用程序配置对象，包含所有关键路径。
     * @return true 如果所有验证都通过。
     * @return false 如果发现任何致命错误（会自动打印错误日志到 stderr）。
     */
    static bool validate_environment(const AppConfig& config);
};

#endif // STARTUP_VALIDATOR_HPP
