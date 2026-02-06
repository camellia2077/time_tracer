// config/config_loader.hpp
#ifndef CONFIG_CONFIG_LOADER_H_
#define CONFIG_CONFIG_LOADER_H_

#include <filesystem>
#include <string>

#include "common/config/app_config.hpp"

class ConfigLoader {
 public:
  /**
   * @brief Constructs a ConfigLoader.
   * @param exe_path_str The path to the executable, typically from argv[0].
   */
  ConfigLoader(const std::string& exe_path_str);

  /**
   * @brief Loads all application configurations from files.
   * It finds and parses the main 'config.json' and then resolves the paths
   * to other configuration files specified within it.
   * @return A populated AppConfig object.
   * @throws std::runtime_error on file-not-found, parsing, or permission
   * errors.
   */
  auto LoadConfiguration() -> AppConfig;

  /**
   * @brief Gets the absolute path to the main config.json file.
   * @return A string containing the full path.
   */
  [[nodiscard]] auto GetMainConfigPath() const -> std::string;

 private:
  std::filesystem::path exe_path_;
  std::filesystem::path config_dir_path_;
  std::filesystem::path main_config_path_;

  // Constants defining key configuration file and directory names
  const std::string kConfigFileName = "config.toml";
  const std::string kConfigDirName = "config";
};

#endif  // CONFIG_CONFIG_LOADER_H_
