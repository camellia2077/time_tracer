// bootstrap/StartupValidator.hpp
#ifndef STARTUP_VALIDATOR_HPP
#define STARTUP_VALIDATOR_HPP

#include "common/config/AppConfig.hpp"

/**
 * @class StartupValidator
 * @brief 负责在应用程序启动前验证运行环境的基础完整性。
 * @details 现在的职责被精简为仅检查必要的运行时依赖（如 DLL 插件）。
 * 配置文件的逻辑验证已移交至各个具体的业务模块或显式的 validate 命令。
 */
class StartupValidator {
public:
    /**
     * @brief 执行环境完整性检查。
     * @param config 应用程序配置对象，用于获取插件目录路径。
     * @return true 如果环境依赖（DLL）完整。
     */
    static bool validate_environment(const AppConfig& config);
};

#endif // STARTUP_VALIDATOR_HPP