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

// Include the nlohmann/json library
#include <nlohmann/json.hpp>

// For Windows-specific console settings
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// Use nlohmann::json
using json = nlohmann::json;

// --- 1. Utility Components ( unchanged ) ---
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

    void print_usage(const char* prog_name) {
        std::cerr << ConsoleColors::red << "Usage: " <<  ConsoleColors::reset << prog_name << " <num_days> <items_per_day>\n";
        std::cerr << "       " << prog_name << " --version\n"; // NEW: Add usage for version
        std::cerr << "Description: Generates test log data. Reads activities from 'activities_config.json'." << '\n';
        std::cerr << "  <num_days>        : Total number of days to generate (positive integer)." << '\n';
        std::cerr << "  <items_per_day>   : Number of log items per day (positive integer)." << '\n';
        std::cerr << "  --version         : Display version information and exit." << '\n'; // NEW: Describe the option
        std::cerr << "Example: " << prog_name << " 10 5" << '\n';
    }
    void print_version() {
        // NEW: Define version constants here for easy updating.
        const std::string APP_VERSION = "1.2.0";
        const std::string LAST_UPDATE = "2025-06-24";
        std::cout << "log_generator version " << APP_VERSION << std::endl;
        std::cout << "Last Updated: " << LAST_UPDATE << std::endl;
    }

} // namespace Utils


// --- 2. Configuration Components ---
// These components are responsible for loading and validating configuration from external sources.

struct Config {
    int num_days;
    int items_per_day;
};

namespace ConfigLoader {

    std::optional<std::vector<std::string>> load_activities_from_json(const std::string& json_filename) {
        std::ifstream f(json_filename);
        if (!f.is_open()) {
            std::cerr << Utils::ConsoleColors::red << "Error: Could not open activities configuration file '" << json_filename << "'." << Utils::ConsoleColors::reset << '\n';
            return std::nullopt;
        }

        try {
            json data = json::parse(f);
            f.close();

            if (data.contains("common_activities") && data["common_activities"].is_array()) {
                auto activities = data["common_activities"].get<std::vector<std::string>>();
                if (activities.empty())
                {
                    std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "\"common_activities\" array in '" << json_filename << "' is empty." << '\n';
                    return std::nullopt;
                }
                std::cout << Utils::ConsoleColors::green << "Successfully loaded " << Utils::ConsoleColors::reset << " activities from '" << json_filename << "'." << '\n';
                return activities;
            }
            else {
                std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON file '" << json_filename << "' must contain a non-empty 'common_activities' array." << '\n';
                return std::nullopt;
            }
        }
        catch (const json::parse_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "Failed to parse JSON from '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const json::type_error& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "JSON type error in '" << json_filename << "'. Is 'common_activities' an array of strings? Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
        catch (const std::exception& e) {
            std::cerr << Utils::ConsoleColors::red << "Error: " << Utils::ConsoleColors::reset << "An unexpected error occurred while processing '" << json_filename << "'. Detail: " << e.what() << '\n';
            if (f.is_open()) f.close();
            return std::nullopt;
        }
    }

    std::optional<Config> parse_arguments(int argc, char* argv[]) {
        if (argc != 3) {
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        Config config;
        try {
            config.num_days = std::stoi(argv[1]);
            config.items_per_day = std::stoi(argv[2]);
        }
        catch (const std::invalid_argument&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Invalid argument. <num_days> and <items_per_day> must be integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }
        catch (const std::out_of_range&) {
            std::cerr << Utils::ConsoleColors::red << "Error: Argument out of range." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }

        if (config.num_days <= 0 || config.items_per_day <= 0) {
            std::cerr << Utils::ConsoleColors::red << "Error: <num_days> and <items_per_day> must be positive integers." << Utils::ConsoleColors::reset << '\n';
            Utils::print_usage(argv[0]);
            return std::nullopt;
        }
        
        // --- MODIFICATION START ---
        // The check below is removed as "睡觉长" is no longer a required, separate item.
        // The positive integer check above is sufficient.
        /*
        if (config.items_per_day < 2) {
            std::cerr << Utils::ConsoleColors::red << "Error: <items_per_day> must be at least 2 to include '起床' and '睡觉长'." << Utils::ConsoleColors::reset << '\n';
            return std::nullopt;
        }
        */
        // --- MODIFICATION END ---

        return config;
    }
} // namespace ConfigLoader


// --- 3. Core Logic Component ---
// This class is dedicated to the business logic of generating log data.
class LogGenerator {
public:
    LogGenerator(const Config& config, const std::vector<std::string>& activities)
        : config_(config),
        common_activities_(activities),
        gen_(std::random_device{}()), // Initialize random number generator
        dis_minute_(0, 59),
        dis_activity_selector_(0, static_cast<int>(activities.size()) - 1) {}

    /**
     * @brief Generates log data and writes it to the provided output stream.
     * @param outStream The stream (e.g., file stream, string stream) to write data to.
     */
    void generate(std::ostream& outStream) {
        static const int days_in_months[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        const int days_per_chunk = 1000;

        std::ostringstream log_chunk_stream;

        int current_month = 1;
        int current_day_of_month = 1;

        int minute_for_todays_actual_wakeup = dis_minute_(gen_);

        for (int d = 0; d < config_.num_days; ++d) {
            if (d > 0) {
                log_chunk_stream << '\n';
            }

            log_chunk_stream << Utils::format_two_digits(current_month) << Utils::format_two_digits(current_day_of_month) << '\n';
            
            // --- MODIFICATION START ---
            // The generation logic for the last item ("睡觉长") is removed.
            // The loop now treats all items after the first one as a regular activity.
            for (int i = 0; i < config_.items_per_day; ++i) {
                int display_hour_final;
                int event_minute_final;
                std::string event_text_to_use_final;

                if (i == 0) {
                    // First item is always "起床"
                    event_text_to_use_final = "起床";
                    display_hour_final = 6;
                    event_minute_final = minute_for_todays_actual_wakeup;
                }
                else {
                    // All other items are now regular, random activities
                    double progress_ratio = static_cast<double>(i) / (config_.items_per_day - 1);
                    int logical_event_hour = 6 + static_cast<int>(std::round(progress_ratio * 19.0));
                    if (logical_event_hour > 25) logical_event_hour = 25;
                    display_hour_final = (logical_event_hour >= 24) ? logical_event_hour - 24 : logical_event_hour;
                    event_minute_final = dis_minute_(gen_);
                    event_text_to_use_final = common_activities_[dis_activity_selector_(gen_)];
                }
                log_chunk_stream << Utils::format_two_digits(display_hour_final) << Utils::format_two_digits(event_minute_final) << event_text_to_use_final << '\n';
            }
            // --- MODIFICATION END ---

            // The wakeup time for the next day is now generated at the start of that day's loop.
            minute_for_todays_actual_wakeup = dis_minute_(gen_); 

            current_day_of_month++;
            if (current_day_of_month > days_in_months[current_month]) {
                current_day_of_month = 1;
                current_month = (current_month % 12) + 1;
            }

            if ((d + 1) % days_per_chunk == 0 || (d + 1) == config_.num_days) {
                outStream << log_chunk_stream.str();
                log_chunk_stream.str("");
                log_chunk_stream.clear();
            }
        }
    }

private:
    const Config& config_;
    const std::vector<std::string>& common_activities_;
    std::mt19937 gen_;
    std::uniform_int_distribution<> dis_minute_;
    std::uniform_int_distribution<> dis_activity_selector_;
};


// --- 4. Application Runner ( unchanged ) ---
class Application {
public:
    int run(int argc, char* argv[]) {
        if (argc == 2 && std::string(argv[1]) == "--version") {
            Utils::print_version();
            return 0; // Exit successfully after printing the version.
        }
        Utils::setup_console();

        // Phase 1: Configuration & Setup
        auto config_opt = ConfigLoader::parse_arguments(argc, argv);
        if (!config_opt) {
            return 1;
        }
        Config config = *config_opt;

        auto activities_opt = ConfigLoader::load_activities_from_json("activities_config.json");
        if (!activities_opt) {
            std::cerr << Utils::ConsoleColors::red << "Exiting program due to configuration loading failure." << Utils::ConsoleColors::reset << std::endl;
            return 1;
        }
        const std::vector<std::string>& common_activities = *activities_opt;

        // Phase 2: Execution & Output
        auto start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Generating data....." << '\n';

        std::ostringstream filename_ss;
        filename_ss << "log_" << config.num_days << "_items_" << config.items_per_day << ".txt";
        std::string output_filename = filename_ss.str();

        std::ofstream outFile(output_filename);
        if (!outFile.is_open()) {
            std::cerr << Utils::ConsoleColors::red << "Error: Could not open file '" << output_filename << "' for writing." << Utils::ConsoleColors::reset << '\n';
            return 1;
        }

        // Create the generator and run the core logic
        LogGenerator generator(config, common_activities);
        generator.generate(outFile);

        outFile.close();

        // Phase 3: Reporting
        auto end_time = std::chrono::high_resolution_clock::now();
        report_completion(output_filename, start_time, end_time);
        
        return 0;
    }

private:
    void report_completion(const std::string& filename, 
                           const std::chrono::high_resolution_clock::time_point& start, 
                           const std::chrono::high_resolution_clock::time_point& end) {
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        auto duration_s = std::chrono::duration<double>(end - start);

        std::cout << Utils::ConsoleColors::green << "Data generation complete. " << Utils::ConsoleColors::reset << "Output is in '" << filename << "'" << '\n';
        std::cout << "Total generation time: " << duration_ms.count() << " ms (" << std::fixed << std::setprecision(3) << duration_s.count() << " s)." << '\n';
    }
};


// --- 5. Main Entry Point ( unchanged ) ---
int main(int argc, char* argv[]) {
    Application app;
    return app.run(argc, argv);
}