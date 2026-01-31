// application/pipeline/utils/converter_config_factory.cpp
#include "converter_config_factory.hpp"

#include <iostream>

#include "config/loader/converter_config_loader.hpp"

namespace fs = std::filesystem;

namespace core::pipeline {

auto ConverterConfigFactory::create(const fs::path& interval_config_path,
                                    const AppConfig& app_config)
    -> ConverterConfig {
  std::cout << "Loading Converter Config from: "
            << interval_config_path.string() << std::endl;

  // 1. 委托 Config 模块加载 (文件读取、合并、Struct填充)
  ConverterConfig config =
      ConverterConfigLoader::load_from_file(interval_config_path);

  // 2. 注入运行时参数 (Initial Top Parents)
  for (const auto& [path_key, path_val] :
       app_config.pipeline.initial_top_parents) {
    config.initial_top_parents[path_key.string()] = path_val.string();
  }

  return config;
}

}  // namespace core::pipeline