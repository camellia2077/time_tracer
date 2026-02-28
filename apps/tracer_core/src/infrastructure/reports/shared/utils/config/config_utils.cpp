// infrastructure/reports/shared/utils/config/config_utils.cpp
#include "infrastructure/reports/shared/utils/config/config_utils.hpp"

#include <fstream>
#include <stdexcept>

auto LoadJsonConfig(const std::filesystem::path& config_path,
                    const std::string& error_message_prefix) -> nlohmann::json {
  std::ifstream config_file(config_path);
  if (!config_file.is_open()) {
    throw std::runtime_error(error_message_prefix + config_path.string());
  }
  nlohmann::json config_data;
  config_file >> config_data;
  return config_data;
}
