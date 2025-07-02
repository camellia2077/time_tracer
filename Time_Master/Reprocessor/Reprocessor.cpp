#include "Reprocessor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp" // Included here for use in modify_bill

// --- Constructor ---

// **MODIFIED**: The constructor now simply stores the directory path.
Reprocessor::Reprocessor(const std::string& config_dir_path) : m_config_dir_path(config_dir_path) {
    // No config loading is done at construction time.
}

// --- Public Methods ---

bool Reprocessor::validate_bill(const std::string& bill_path) {
    try {
        // **MODIFIED**: Construct the full path to the validator's config file.
        // Note: Using '/' as a path separator works on Windows, Linux, and macOS.
        const std::string validator_config_path = m_config_dir_path + "/Validator_Config.json";
        
        BillValidator validator(validator_config_path); 
        
        std::cout << "\n--- Starting Validation using '" << validator_config_path << "' ---\n";
        bool result = validator.validate(bill_path);
        if (result) {
            std::cout << "Validation successful: No errors found. (Warnings may still be present)\n";
        } else {
            std::cerr << "Validation failed: Errors were found.\n";
        }
        std::cout << "--- Validation Finished ---\n";
        return result;

    } catch (const std::runtime_error& e) {
        std::cerr << "A critical error occurred during validation setup: " << e.what() << std::endl;
        return false;
    }
}

bool Reprocessor::modify_bill(const std::string& input_bill_path, const std::string& output_bill_path) {
    try {
        // **MODIFIED**: Load the modifier's specific configuration file on the fly.
        const std::string modifier_config_path = m_config_dir_path + "/Modifier_Config.json";
        std::ifstream config_file(modifier_config_path);
        if (!config_file.is_open()) {
            throw std::runtime_error("Error: Could not open modifier config file '" + modifier_config_path + "'");
        }
        
        nlohmann::json modifier_json;
        try {
            config_file >> modifier_json;
        } catch (nlohmann::json::parse_error& e) {
            throw std::runtime_error("Error: Failed to parse JSON from '" + modifier_config_path + "': " + std::string(e.what()));
        }
        config_file.close();


        // 1. Read the input bill file.
        std::ifstream input_file(input_bill_path);
        if (!input_file.is_open()) {
            std::cerr << "Error: Could not open input bill file '" << input_bill_path << "'\n";
            return false;
        }
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        std::string bill_content = buffer.str();
        input_file.close();

        // 2. Create a modifier with the just-loaded JSON config and modify the content.
        BillModifier modifier(modifier_json);
        std::cout << "\n--- Starting Modification using '" << modifier_config_path << "' ---\n";
        std::string modified_content = modifier.modify(bill_content);

        // 3. Write the modified content to the output file.
        std::ofstream output_file(output_bill_path);
        if (!output_file.is_open()) {
            std::cerr << "Error: Could not open output bill file '" << output_bill_path << "' for writing.\n";
            return false;
        }
        output_file << modified_content;
        output_file.close();
        
        std::cout << "--- Modification successful. Output saved to '" << output_bill_path << "' ---\n";
        return true;

    } catch (const std::exception& e) {
        std::cerr << "An error occurred during modification: " << e.what() << std::endl;
        return false;
    }
}