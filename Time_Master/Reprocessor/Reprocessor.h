#ifndef REPROCESSOR_H
#define REPROCESSOR_H

#include <string>
#include "BillValidator.h"
#include "BillModifier.h"
// nlohmann/json.hpp is no longer needed in the header, it's a detail of the .cpp file now.

class Reprocessor {
public:
    /**
     * @brief Constructs a Reprocessor instance.
     * @param config_dir_path Path to the directory containing configuration files.
     */
    explicit Reprocessor(const std::string& config_dir_path);

    /**
     * @brief Validates a bill file using config/Validator_Config.json.
     * @param bill_path Path to the bill file to validate.
     * @return True if validation passes (no errors), false otherwise.
     */
    bool validate_bill(const std::string& bill_path);

    /**
     * @brief Modifies a bill file using config/Modifier_Config.json.
     * @param input_bill_path Path to the source bill file.
     * @param output_bill_path Path where the modified bill will be saved.
     * @return True on success, false if file operations fail.
     */
    bool modify_bill(const std::string& input_bill_path, const std::string& output_bill_path);

private:
    // **MODIFIED**: Store the directory path, not a specific file path or JSON object.
    std::string m_config_dir_path;
};

#endif // REPROCESSOR_H