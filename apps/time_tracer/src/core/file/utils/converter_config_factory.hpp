// core/file/utils/converter_config_factory.hpp
#ifndef CORE_FILE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_
#define CORE_FILE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_

#include <filesystem>
#include "converter/config/converter_config.hpp"
#include "common/config/app_config.hpp"

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

#endif // CORE_FILE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_