// application/pipeline/utils/converter_config_factory.hpp
#ifndef APPLICATION_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_H_
#define APPLICATION_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_H_

#include <filesystem>
// [修复] 移除错误的自包含
// #include "application/pipeline/utils/converter_config_factory.hpp"

#include "infrastructure/config/models/app_config.hpp"
#include "infrastructure/config/models/converter_config_models.hpp"

namespace core::pipeline {

class ConverterConfigFactory {
 public:
  static auto Create(const std::filesystem::path& interval_config_path,
                     const AppConfig& app_config) -> ConverterConfig;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_UTILS_CONVERTER_CONFIG_FACTORY_H_