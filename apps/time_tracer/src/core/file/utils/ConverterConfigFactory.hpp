// core/file/utils/ConverterConfigFactory.hpp
#ifndef CONVERTER_CONFIG_FACTORY_HPP
#define CONVERTER_CONFIG_FACTORY_HPP

#include <filesystem>
#include "converter/config/ConverterConfig.hpp"
#include "common/config/AppConfig.hpp"

class ConverterConfigFactory {
public:
    /**
     * @brief 加载合并后的 JSON 并构建完整的 ConverterConfig
     * @param interval_config_path 主配置文件路径
     * @param app_config 全局 AppConfig (用于注入 initial_top_parents)
     * @return 构造好的配置对象
     */
    static ConverterConfig create(const std::filesystem::path& interval_config_path, 
                                  const AppConfig& app_config);
};

#endif // CONVERTER_CONFIG_FACTORY_HPP