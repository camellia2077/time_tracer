// infrastructure/config/file_converter_config_provider.cpp
#include "infrastructure/config/file_converter_config_provider.hpp"

#include <stdexcept>
#include <utility>

#include "infrastructure/config/loader/converter_config_loader.hpp"

namespace infrastructure::config {

FileConverterConfigProvider::FileConverterConfigProvider(
    std::filesystem::path config_path,
    std::unordered_map<std::filesystem::path, std::filesystem::path>
        initial_top_parents)
    : config_path_(std::move(config_path)),
      initial_top_parents_(std::move(initial_top_parents)) {}

auto FileConverterConfigProvider::LoadConverterConfig() const
    -> ConverterConfig {
  if (config_path_.empty()) {
    throw std::runtime_error("Converter config path is empty.");
  }

  if (!cached_config_.has_value()) {
    ConverterConfig config = ConverterConfigLoader::LoadFromFile(config_path_);
    for (const auto& [key, val] : initial_top_parents_) {
      config.initial_top_parents[key.string()] = val.string();
    }
    cached_config_ = std::move(config);
  }

  return *cached_config_;
}

}  // namespace infrastructure::config
