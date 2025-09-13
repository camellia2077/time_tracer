// file_handler/ConfigLoader.hpp

#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <filesystem>
#include "common/AppConfig.hpp"

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
     * @throws std::runtime_error on file-not-found, parsing, or permission errors.
     */
    AppConfig load_configuration();

    /**
     * @brief Gets the absolute path to the main config.json file.
     * @return A string containing the full path.
     */
    std::string get_main_config_path() const;

private:
    std::filesystem::path exe_path;
    std::filesystem::path config_dir_path;
    std::filesystem::path main_config_path;

    // Constants defining key configuration file and directory names
    const std::string CONFIG_FILE_NAME = "config.json";
    const std::string CONFIG_DIR_NAME = "config";
};

#endif // CONFIG_LOADER_HPP