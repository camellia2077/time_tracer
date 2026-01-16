// config/ConfigFacade.hpp
#ifndef CONFIG_FACADE_HPP
#define CONFIG_FACADE_HPP

#include "Config.hpp"
#include <vector>
#include <string>

namespace ConfigFacade {
    /**
     * @brief 验证加载后的JSON配置数据。
     * @param config_data 要验证的配置数据。
     * @param errors 用于收集错误信息的字符串向量。
     * @return 如果验证通过，返回true；否则返回false。
     */
    bool validate(const TomlConfigData& config_data, std::vector<std::string>& errors);
}

#endif // CONFIG_FACADE_HPP