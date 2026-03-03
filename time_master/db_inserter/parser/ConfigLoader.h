#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <string>
#include "ParserConfig.h" // MODIFIED: Include path changed

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

#endif // CONFIG_LOADER_H