#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include "ParserConfig.hpp" 

/**
 * @class ConfigLoader
 * @brief A utility class for loading configuration from a JSON file.
 * @details This class is responsible for all file I/O and JSON parsing logic.
 */
class ConfigLoader {
public:
    /**
     * @brief Loads, parses, and validates the JSON config file.
     * @param config_path The path to the JSON configuration file.
     * @return A ParserConfig object populated with data from the file.
     */
    static ParserConfig load_from_file(const std::string& config_path); // MODIFIED: Return type changed
};

#endif // CONFIG_LOADER_HPP