#include "common/pch.h"
#include "ConfigLoader.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

ParserConfig ConfigLoader::load_from_file(const std::string& config_path) {
    ParserConfig config; 
    
    std::ifstream config_stream(config_path);
    if (!config_stream.is_open()) {
        std::cerr << "Warning: Could not open main config file: " << config_path 
                  << ". Proceeding without initial parent mappings." << std::endl;
        return config;
    }

    nlohmann::json config_json;
    try {
        config_stream >> config_json;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing config file: " << config_path << " - " << e.what() << std::endl;
        return config;
    }

    try {
        if (config_json.contains("initial_top_level_parents") && config_json["initial_top_level_parents"].is_object()) {
            const auto& parents_map = config_json["initial_top_level_parents"];
            for (auto& [key, value] : parents_map.items()) {
                if (value.is_string()) {
                    config.initial_top_level_parents[key] = value.get<std::string>();
                }
            }
        } else {
            std::cerr << "Warning: Provided JSON config does not contain a valid 'initial_top_level_parents' object. Proceeding without initial parent mappings." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Warning: Error processing provided JSON config. Details: " << e.what() << std::endl;
    }

    return config;
}