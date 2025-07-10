#include "Config.h"
#include "LogGenerator.h"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <random>
#include <cmath>
#include <memory>
#include <chrono>
#include <optional>
#include <filesystem>
#include <format>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- NEW: Utils namespace is now fully contained in main.cpp ---
namespace Utils {
    // Declarations from former Utils.h
    struct ConsoleColors {
        static const std::string red;
        static const std::string green;
        static const std::string reset;
    };
    void setup_console();
    bool is_leap(int year);
    int get_days_in_month(int year, int month);
    void print_usage(const char* prog_name);
    void print_version();

    // Implementations from former main.cpp's Utils section
    const std::string ConsoleColors::red = "\033[1;31m";
    const std::string ConsoleColors::green = "\033[1;32m";
    const std::string ConsoleColors::reset = "\033[0m";

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
    
    bool is_leap(int year) {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    int get_days_in_month(int year, int month) {
        if (month < 1 || month > 12) return 0;
        if (month == 2) {
            return is_leap(year) ? 29 : 28;
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            return 30;
        } else {
            return 31;
        }
    }

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
        const std::string APP_VERSION = "2.2.0";
        const std::string LAST_UPDATE = "2025-06-27";
        std::cout << "log_generator version " << APP_VERSION << std::endl;
        std::cout << "Last Updated: " << LAST_UPDATE << std::endl;
    }
} // namespace Utils

// --- Application Runner ---
class Application {
    public:
        int run(int argc, char* argv[]) {
            if (argc == 2 && std::string(argv[1]) == "--version") {
                Utils::print_version();
                return 0;
            }
            Utils::setup_console();
    
            auto config_opt = ConfigLoader::parse_arguments(argc, argv);
            if (!config_opt) {
                return 1;
            }
            Config config = *config_opt;
    
            auto json_configs_opt = ConfigLoader::load_json_configurations("activities_config.json");
            if (!json_configs_opt) {
                std::cerr << Utils::ConsoleColors::red << "Exiting program due to configuration loading failure." << Utils::ConsoleColors::reset << std::endl;
                return 1;
            }
    
            auto total_start_time = std::chrono::high_resolution_clock::now();
            std::cout << "Generating data for years " << config.start_year << " to " << config.end_year << "..." << '\n';
    
            LogGenerator generator(config.items_per_day, json_configs_opt->activities, json_configs_opt->remarks);
            int files_generated = 0;
            
            // NEW: Durations to store timings for different phases
            auto total_generation_duration = std::chrono::nanoseconds(0);
            auto total_io_duration = std::chrono::nanoseconds(0);
    
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
                std::filesystem::path year_dir_path = std::filesystem::path(master_dir_name) / std::to_string(year);
                
                try {
                    if (!std::filesystem::exists(year_dir_path)) {
                        std::filesystem::create_directory(year_dir_path);
                    }
                } catch(const std::filesystem::filesystem_error& e) {
                    std::cerr << Utils::ConsoleColors::red << "Error creating directory '" << year_dir_path.string() << "'. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n';
                    return 1;
                }
    
                for (int month = 1; month <= 12; ++month) {
                    std::string filename = std::format("{}_{:02}.txt", year, month);
                    std::filesystem::path full_path = year_dir_path / filename;
    
                    // --- MODIFIED: Timing logic implemented ---
                    // 1. Time the text generation
                    auto gen_start = std::chrono::high_resolution_clock::now();
                    int days_in_month = Utils::get_days_in_month(year, month);
                    std::string month_log = generator.generate_for_month(month, days_in_month); // Now returns string
                    auto gen_end = std::chrono::high_resolution_clock::now();
                    total_generation_duration += (gen_end - gen_start);

                    // 2. Time the file I/O
                    auto io_start = std::chrono::high_resolution_clock::now();
                    std::ofstream outFile(full_path);
                    if (!outFile.is_open()) {
                        std::cerr << Utils::ConsoleColors::red << "Error: Could not open file '" << full_path.string() << "' for writing." << Utils::ConsoleColors::reset << '\n';
                        continue;
                    }
                    outFile << month_log;
                    outFile.close();
                    auto io_end = std::chrono::high_resolution_clock::now();
                    total_io_duration += (io_end - io_start);

                    files_generated++;
                }
            }
    
            auto total_end_time = std::chrono::high_resolution_clock::now();
            report_completion(config, files_generated, total_start_time, total_end_time, total_generation_duration, total_io_duration);
            
            return 0;
        }
    
    private:
        // MODIFIED: Added duration parameters for detailed reporting
        void report_completion(const Config& config,
                               int files_generated,
                               const std::chrono::high_resolution_clock::time_point& start, 
                               const std::chrono::high_resolution_clock::time_point& end,
                               const std::chrono::nanoseconds& gen_duration,
                               const std::chrono::nanoseconds& io_duration) {

            auto total_duration = end - start;
            
            // Convert all durations to seconds (double) and milliseconds (long long)
            auto total_s = std::chrono::duration<double>(total_duration);
            auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);

            auto gen_s = std::chrono::duration<double>(gen_duration);
            auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(gen_duration);

            auto io_s = std::chrono::duration<double>(io_duration);
            auto io_ms = std::chrono::duration_cast<std::chrono::milliseconds>(io_duration);

            std::cout << std::format("{}\nData generation complete.{} {} monthly log files created for years {}-{}.\n",
                                     Utils::ConsoleColors::green,
                                     Utils::ConsoleColors::reset,
                                     files_generated,
                                     config.start_year,
                                     config.end_year);
            
            // NEW: Print timing details in the requested format
            std::cout << "---------------------------\n";
            std::cout << std::format("total time:    {:.2f} s ({}ms)\n", total_s.count(), total_ms.count());
            std::cout << std::format("text generate: {:.2f} s ({}ms)\n", gen_s.count(), gen_ms.count());
            std::cout << std::format("io:            {:.2f} s ({}ms)\n", io_s.count(), io_ms.count());
            std::cout << "---------------------------\n";
        }
    };

// --- Main Entry Point ---
int main(int argc, char* argv[]) {
    Application app;
    return app.run(argc, argv);
}