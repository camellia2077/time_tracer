#include <iostream>
#include <vector>
#include <string>
#include <iomanip>     // For std::setw, std::setfill
#include <sstream>     // For std::ostringstream, std::to_string
#include <fstream>     // For std::ofstream, std::ifstream
#include <stdexcept>   // For std::invalid_argument, std::out_of_range
#include <random>      // For random number generation
#include <cmath>       // For std::round
#include <memory>      // For std::unique_ptr
#include <chrono>      // For timing the execution
#include <optional>    // For safely returning from argument parsing and file loading
#include <filesystem>  // MODIFIED: Added for directory creation

// Include the nlohmann/json library
#include <nlohmann/json.hpp>

// For Windows-specific console settings
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Use nlohmann::json
using json = nlohmann::json;

// --- 1. Utility Components ---
// These are general-purpose tools that can be used throughout the application.

namespace Utils {

    // Structure to hold ANSI color codes for console output.
    struct ConsoleColors {
        static const std::string red;
        static const std::string green;
        static const std::string reset;
    };

    const std::string ConsoleColors::red = "\033[1;31m";
    const std::string ConsoleColors::green = "\033[1;32m";
    const std::string ConsoleColors::reset = "\033[0m";

    // Sets up the console for UTF-8 and enables ANSI color codes on Windows.
    void setup_console() {
    #if defined(_WIN32) || defined(_WIN64)
        SetConsoleOutputCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
    #endif
    }

    std::string format_two_digits(int n) {
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << n;
        return ss.str();
    }
    
    // MODIFIED: Add helper functions for date calculations
    bool is_leap(int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    int get_days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0; // Basic validation
        if (month == 2) {
            return is_leap(year) ? 29 : 28;
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            return 30;
        } else {
            return 31;
        }
    }

    // MODIFIED: Updated usage instructions for new arguments.
    void print_usage(const char* prog_name) {
        std::cerr << ConsoleColors::red << "Usage: " << ConsoleColors::reset << prog_name << " <start_year> <end_year> <items_per_day>\n";
        std::cerr << "       " << prog_name << " --version\n";
        std::cerr << "Description: Generates test log data for a given year range. Reads activities from 'activities_config.json'.\n";
        std::cerr << "  <start_year>      : The starting year (e.g., 1990).\n";
        std::cerr << "  <end_year>        : The ending year (inclusive).\n";
        std::cerr << "  <items_per_day>   : Number of log items per day (positive integer).\n";
        std::cerr << "  --version         : Display version information and exit.\n";
        std::cerr << "Example: " << prog_name << " 2023 2024 5\n";
    }
    void print_version() {
        const std::string APP_VERSION = "2.2.0"; // MODIFIED: Version bump for new feature
        const std::string LAST_UPDATE = "2025-06-27";
        std::cout << "log_generator version " << APP_VERSION << std::endl;
        std::cout << "Last Updated: " << LAST_UPDATE << std::endl;
    }

} // namespace Utils


// --- 2. Configuration Components ---
// These components are responsible for loading and validating configuration from external sources.

// MODIFIED: Structure for daily remarks now includes generation probability.
struct DailyRemarkConfig {
    std::string prefix;
    std::vector<std::string> contents;
    double generation_chance = 0.5; // Default to 50%
};

// MODIFIED: Config struct updated for year range.
struct Config {
    int start_year;
    int end_year;
    int items_per_day;
};

// NEW: Structure to hold all configurations loaded from the JSON file.
struct JsonConfigData {
    std::vector<std::string> activities;
    std::optional<DailyRemarkConfig> remarks;
};

namespace ConfigLoader {

    // MODIFIED: This function now loads all relevant data from the JSON file.
    std::optional<JsonConfigData> load_json_configurations(const std::string& json_filename) {
        std::ifstream f(json_filename);
        if (!f.is_open()) {
            std::cerr << Utils::ConsoleColors::red << "Error: Could not open configuration file '" << json_filename << "'." << Utils::ConsoleColors::reset << '\n';
            return std::nullopt;
        }

        try {
            json data = json::parse(f);
            f.close();

            JsonConfigData config_data;

            // 1. Load "common_activities" (mandatory)
            if (data.contains("common_activities") && data["common_activities"].is_array() && !data["common_activities"].empty()) {
                config_data.activities = data["common_activities"].get<std::vector<std::string>>();
                std::cout << Utils::ConsoleColors::green << "Successfully loaded " << Utils::ConsoleColors::reset << config_data.activities.size() << " activities from '" << json_filename << "'." << '\n';
            } else {
                std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON file '" << json_filename << "' must contain a non-empty 'common_activities' array." << '\n';
                return std::nullopt;
            }

            // 2. Load "daily_remarks" (optional)
            if (data.contains("daily_remarks") && data["daily_remarks"].is_object()) {
                const auto& remarks_json = data["daily_remarks"];
                DailyRemarkConfig remarks;
                bool prefix_ok = false;
                bool contents_ok = false;

                if (remarks_json.contains("prefix") && remarks_json["prefix"].is_string()) {
                    remarks.prefix = remarks_json["prefix"].get<std::string>();
                    prefix_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << json_filename << "' is missing a 'prefix' string. This feature will be disabled.\n";
                }

                if (remarks_json.contains("contents") && remarks_json["contents"].is_array() && !remarks_json["contents"].empty()) {
                    remarks.contents = remarks_json["contents"].get<std::vector<std::string>>();
                    contents_ok = true;
                } else {
                    std::cerr << "Warning: 'daily_remarks' object in '" << json_filename << "' is missing a non-empty 'contents' array. This feature will be disabled.\n";
                }

                // NEW: Load "generation_chance" (optional)
                if (remarks_json.contains("generation_chance") && remarks_json["generation_chance"].is_number()) {
                    double chance = remarks_json["generation_chance"].get<double>();
                    if (chance >= 0.0 && chance <= 1.0) {
                        remarks.generation_chance = chance;
                    } else {
                        std::cerr << "Warning: 'generation_chance' in '" << json_filename << "' must be between 0.0 and 1.0. Using default of " << remarks.generation_chance << ".\n";
                    }
                }

                // Only emplace if both prefix and contents were valid
                if (prefix_ok && contents_ok) {
                    config_data.remarks.emplace(remarks);
                    std::cout << Utils::ConsoleColors::green << "Successfully loaded " << Utils::ConsoleColors::reset << remarks.contents.size() << " daily remarks with a " << (remarks.generation_chance * 100) << "% generation chance." << '\n';
                }
            }

            return config_data;
        }
        catch (const json::parse_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "Failed to parse JSON from '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const json::type_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON type error in '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const std::exception& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "An unexpected error occurred while processing '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
    }


    // MODIFIED: Argument parsing logic completely rewritten for new requirements.
    std::optional<Config> parse_arguments(int argc, char* argv[]) {
        if (argc != 4) {
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        Config config;
        try {
            config.start_year = std::stoi(argv[1]);
            config.end_year = std::stoi(argv[2]);
            config.items_per_day = std::stoi(argv[3]);
        }
        catch (const std::invalid_argument&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Invalid argument. All arguments must be integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }
        catch (const std::out_of_range&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Argument out of range." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        if (config.start_year <= 0 || config.end_year <= 0 || config.items_per_day <= 0) {
            std::cerr << Utils::ConsoleColors::red << "Error: Years and <items_per_day> must be positive integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        if (config.end_year < config.start_year) {
            std::cerr << Utils::ConsoleColors::red << "Error: <end_year> cannot be earlier than <start_year>." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        return config;
    }
} // namespace ConfigLoader


// --- 3. Core Logic Component ---
// This class is dedicated to the business logic of generating log data.
class LogGenerator {
public:
    // MODIFIED: Constructor now accepts optional remark configuration.
    LogGenerator(int items_per_day,
                 const std::vector<std::string>& activities,
                 const std::optional<DailyRemarkConfig>& remark_config)
        : items_per_day_(items_per_day),
        common_activities_(activities),
        remark_config_(remark_config), // Store the remarks config
        gen_(std::random_device{}()), // Initialize random number generator
        dis_minute_(0, 59),
        dis_activity_selector_(0, static_cast<int>(activities.size()) - 1) {
    
        // Initialize the remark selector and probability distribution only if remarks are configured.
        if (remark_config_ && !remark_config_->contents.empty()) {
            dis_remark_selector_ = std::make_unique<std::uniform_int_distribution<>>(
                0, static_cast<int>(remark_config_->contents.size()) - 1
            );
            // NEW: Initialize a Bernoulli distribution for the generation chance.
            dis_remark_should_generate_ = std::make_unique<std::bernoulli_distribution>(remark_config_->generation_chance);
        }
    }


    /**
     * @brief Generates log data for a full month and writes it to the output stream.
     * @param outStream The stream to write data to.
     * @param month The current month (1-12).
     * @param days_in_month The number of days in the given month.
     */
    // MODIFIED: New generation method for a single month.
    void generate_for_month(std::ostream& outStream, int month, int days_in_month) {
        std::ostringstream log_stream;

        for (int day = 1; day <= days_in_month; ++day) {
            if (day > 1) {
                log_stream << '\n';
            }

            log_stream << Utils::format_two_digits(month) << Utils::format_two_digits(day) << '\n';
            
            // MODIFIED: Add the daily remark line if configured AND the probability check passes.
            if (remark_config_ && dis_remark_selector_ && dis_remark_should_generate_ && (*dis_remark_should_generate_)(gen_)) {
                const std::string& random_content = remark_config_->contents[(*dis_remark_selector_)(gen_)];
                log_stream << remark_config_->prefix << random_content << '\n';
            }
            
            // This daily generation logic is preserved from the original version.
            for (int i = 0; i < items_per_day_; ++i) {
                int display_hour_final;
                int event_minute_final;
                std::string event_text_to_use_final;

                if (i == 0) {
                    event_text_to_use_final = "起床";
                    display_hour_final = 6;
                    event_minute_final = dis_minute_(gen_);
                }
                else {
                    double progress_ratio = (items_per_day_ > 1) ? static_cast<double>(i) / (items_per_day_ - 1) : 1.0;
                    int logical_event_hour = 6 + static_cast<int>(std::round(progress_ratio * 19.0));
                    if (logical_event_hour > 25) logical_event_hour = 25;
                    display_hour_final = (logical_event_hour >= 24) ? logical_event_hour - 24 : logical_event_hour;
                    event_minute_final = dis_minute_(gen_);
                    event_text_to_use_final = common_activities_[dis_activity_selector_(gen_)];
                }
                log_stream << Utils::format_two_digits(display_hour_final) << Utils::format_two_digits(event_minute_final) << event_text_to_use_final << '\n';
            }
        }
        // Write the generated content for the whole month at once.
        outStream << log_stream.str();
    }

private:
    int items_per_day_;
    const std::vector<std::string>& common_activities_;
    const std::optional<DailyRemarkConfig> remark_config_; // Store the config
    std::mt19937 gen_;
    std::uniform_int_distribution<> dis_minute_;
    std::uniform_int_distribution<> dis_activity_selector_;
    // Use a unique_ptr for the optional random distribution for remarks
    std::unique_ptr<std::uniform_int_distribution<>> dis_remark_selector_;
    // NEW: Use a unique_ptr for the optional Bernoulli distribution for remark generation probability
    std::unique_ptr<std::bernoulli_distribution> dis_remark_should_generate_;
};

// --- 4. Application Runner ---
class Application {
    public:
        int run(int argc, char* argv[]) {
            if (argc == 2 && std::string(argv[1]) == "--version") {
                Utils::print_version();
                return 0;
            }
            Utils::setup_console();
    
            // Phase 1: Configuration & Setup
            auto config_opt = ConfigLoader::parse_arguments(argc, argv);
            if (!config_opt) {
                return 1;
            }
            Config config = *config_opt;
    
            // MODIFIED: Call the new JSON configuration loader.
            auto json_configs_opt = ConfigLoader::load_json_configurations("activities_config.json");
            if (!json_configs_opt) {
                std::cerr << Utils::ConsoleColors::red << "Exiting program due to configuration loading failure." << Utils::ConsoleColors::reset << std::endl;
                return 1;
            }
    
            // Phase 2: Execution & Output
            auto start_time = std::chrono::high_resolution_clock::now();
            std::cout << "Generating data for years " << config.start_year << " to " << config.end_year << "..." << '\n';
    
            // MODIFIED: Pass all loaded configs to the generator.
            LogGenerator generator(config.items_per_day, json_configs_opt->activities, json_configs_opt->remarks);
            int files_generated = 0;
    
            // NEW: Define and create the master "Date" directory.
            const std::string master_dir_name = "Date";
            try {
                if (!std::filesystem::exists(master_dir_name)) {
                    std::filesystem::create_directory(master_dir_name);
                    std::cout << "Created master directory: '" << master_dir_name << "'\n";
                }
            } catch(const std::filesystem::filesystem_error& e) {
                std::cerr << Utils::ConsoleColors::red << "Error creating master directory '" << master_dir_name << "'. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n';
                return 1;
            }
            
            for (int year = config.start_year; year <= config.end_year; ++year) {
                // NEW: Construct the path for the year directory inside the master "Date" directory.
                std::filesystem::path year_dir_path = std::filesystem::path(master_dir_name) / std::to_string(year);
                
                try {
                    if (!std::filesystem::exists(year_dir_path)) {
                        std::filesystem::create_directory(year_dir_path);
                        std::cout << "Created directory: '" << year_dir_path.string() << "'\n";
                    }
                } catch(const std::filesystem::filesystem_error& e) {
                    std::cerr << Utils::ConsoleColors::red << "Error creating directory '" << year_dir_path.string() << "'. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n';
                    return 1; // Stop if we can't create a required directory
                }
    
                for (int month = 1; month <= 12; ++month) {
                    std::string filename = std::to_string(year) + "_" + Utils::format_two_digits(month) + ".txt";
    
                    // NEW: Use the 'year_dir_path' object to build the full path.
                    // The path is now correctly built as "Date/YYYY/YYYY_MM.txt"
                    std::filesystem::path full_path = year_dir_path / filename;
    
                    std::ofstream outFile(full_path);
                    if (!outFile.is_open()) {
                        std::cerr << Utils::ConsoleColors::red << "Error: Could not open file '" << full_path.string() << "' for writing." << Utils::ConsoleColors::reset << '\n';
                        continue; // Skip this month and continue with the next
                    }
    
                    int days_in_month = Utils::get_days_in_month(year, month);
                    generator.generate_for_month(outFile, month, days_in_month);
                    outFile.close();
                    files_generated++;
                }
            }
    
            // Phase 3: Reporting
            auto end_time = std::chrono::high_resolution_clock::now();
            report_completion(config, files_generated, start_time, end_time);
            
            return 0;
        }
    
    private:
        // Reporting function is unchanged, but included here for completeness of the Application class
        void report_completion(const Config& config,
                               int files_generated,
                               const std::chrono::high_resolution_clock::time_point& start, 
                               const std::chrono::high_resolution_clock::time_point& end) {
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            auto duration_s = std::chrono::duration<double>(end - start);
    
            std::cout << Utils::ConsoleColors::green << "\nData generation complete. " << Utils::ConsoleColors::reset 
                      << files_generated << " monthly log files created for years " << config.start_year << "-" << config.end_year << "." << '\n';
            std::cout << "Total generation time: " << duration_ms.count() << " ms (" << std::fixed << std::setprecision(3) << duration_s.count() << " s)." << '\n';
        }
    };


// --- 5. Main Entry Point ---
int main(int argc, char* argv[]) {
    Application app;
    return app.run(argc, argv);
}