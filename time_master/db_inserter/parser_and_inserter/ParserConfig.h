#ifndef PARSER_CONFIG_H
#define PARSER_CONFIG_H

#include <string>
#include <map>

/**
 * @struct ParserConfig
 * @brief Holds configuration data specifically for the DataFileParser.
 */
struct ParserConfig {
    std::map<std::string, std::string> initial_top_level_parents;
};

#endif // PARSER_CONFIG_H