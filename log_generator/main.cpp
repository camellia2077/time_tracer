#include "Config.h"         // 包含配置模块
#include "LogGenerator.h"   // 包含核心逻辑模块
#include "Utils.h"          // 包含工具声明

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <random>
#include <cmath>
#include <memory>
#include <chrono>
#include <optional>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- 1. Utility Components (实现) ---
// 在这里，我们只实现 Utils.h 中声明的函数和定义的静态变量
namespace Utils {

    // 【已删除】struct ConsoleColors 的重复定义
    
    // 为在 Utils.h 中声明的静态变量提供定义
    const std::string ConsoleColors::red = "\033[1;31m";
    const std::string ConsoleColors::green = "\033[1;32m";
    const std::string ConsoleColors::reset = "\033[0m";

    // 实现 Utils.h 中声明的函数
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