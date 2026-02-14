// application/bootstrap/startup_validator.hpp
#ifndef APPLICATION_BOOTSTRAP_STARTUP_VALIDATOR_H_
#define APPLICATION_BOOTSTRAP_STARTUP_VALIDATOR_H_

#include "application/dto/runtime_environment_requirements.hpp"

namespace time_tracer::application::ports {
class IRuntimeEnvironmentValidator;
}

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
   * @param requirements 运行时依赖需求描述。
   * @param validator 运行时依赖校验器接口。
   * @return true 如果环境依赖（DLL）完整。
   */
  static auto ValidateEnvironment(
      const time_tracer::application::dto::RuntimeEnvironmentRequirements&
          requirements,
      const time_tracer::application::ports::IRuntimeEnvironmentValidator&
          validator) -> bool;
};

#endif  // APPLICATION_BOOTSTRAP_STARTUP_VALIDATOR_H_
