// core/pipeline/utils/converter_config_factory.hpp
#ifndef CORE_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_
#define CORE_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_

#include <filesystem>
// [修复] 移除错误的自包含
// #include "converter_config_factory.hpp" 

#include "common/config/models/converter_config_models.hpp"
#include "common/config/app_config.hpp"

namespace core::pipeline {

class ConverterConfigFactory {
public:
    static ConverterConfig create(const std::filesystem::path& interval_config_path, 
                                  const AppConfig& app_config);
};

} // namespace core::pipeline

#endif // CORE_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_HPP_