// infrastructure/config/file_converter_config_provider.hpp
#ifndef INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_
#define INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "application/ports/i_converter_config_provider.hpp"

namespace infrastructure::config {

class FileConverterConfigProvider final
    : public time_tracer::application::ports::IConverterConfigProvider {
 public:
  FileConverterConfigProvider(
      std::filesystem::path config_path,
      std::unordered_map<std::filesystem::path, std::filesystem::path>
          initial_top_parents);

  [[nodiscard]] auto LoadConverterConfig() const -> ConverterConfig override;

 private:
  std::filesystem::path config_path_;
  std::unordered_map<std::filesystem::path, std::filesystem::path>
      initial_top_parents_;
  mutable std::optional<ConverterConfig> cached_config_;
};

}  // namespace infrastructure::config

#endif  // INFRASTRUCTURE_CONFIG_FILE_CONVERTER_CONFIG_PROVIDER_H_
