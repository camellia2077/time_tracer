#include "Config.h"
#include "LogGenerator.h"
#include "Utils.h"

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
#include <format> // NEW: 包含 <format> 头文件

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// --- 1. Utility Components (实现) ---
namespace Utils {
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

    // REMOVED: format_two_digits 的实现已被移除
    
    bool is_leap(int year) { //
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); //
    }

    int get_days_in_month(int year, int month) { //
        if (month < 1 || month > 12) return 0; //
        if (month == 2) { //
            return is_leap(year) ? 29 : 28; //
        } else if (month == 4 || month == 6 || month == 9 || month == 11) { //
            return 30; //
        } else {
            return 31; //
        }
    }

    void print_usage(const char* prog_name) { //
        std::cerr << ConsoleColors::red << "Usage: " << ConsoleColors::reset << prog_name << " <start_year> <end_year> <items_per_day>\n"; //
        std::cerr << "       " << prog_name << " --version\n"; //
        std::cerr << "Description: Generates test log data for a given year range. Reads activities from 'activities_config.json'.\n"; //
        std::cerr << "  <start_year>      : The starting year (e.g., 1990).\n"; //
        std::cerr << "  <end_year>        : The ending year (inclusive).\n"; //
        std::cerr << "  <items_per_day>   : Number of log items per day (positive integer).\n"; //
        std::cerr << "  --version         : Display version information and exit.\n"; //
        std::cerr << "Example: " << prog_name << " 2023 2024 5\n"; //
    }
    void print_version() { //
        const std::string APP_VERSION = "2.2.0"; //
        const std::string LAST_UPDATE = "2025-06-27"; //
        std::cout << "log_generator version " << APP_VERSION << std::endl; //
        std::cout << "Last Updated: " << LAST_UPDATE << std::endl; //
    }

} // namespace Utils

// --- 4. Application Runner ---
class Application {
    public:
        int run(int argc, char* argv[]) {
            if (argc == 2 && std::string(argv[1]) == "--version") { //
                Utils::print_version(); //
                return 0; //
            }
            Utils::setup_console(); //
    
            auto config_opt = ConfigLoader::parse_arguments(argc, argv); //
            if (!config_opt) { //
                return 1; //
            }
            Config config = *config_opt; //
    
            auto json_configs_opt = ConfigLoader::load_json_configurations("activities_config.json"); //
            if (!json_configs_opt) { //
                std::cerr << Utils::ConsoleColors::red << "Exiting program due to configuration loading failure." << Utils::ConsoleColors::reset << std::endl; //
                return 1; //
            }
    
            auto start_time = std::chrono::high_resolution_clock::now(); //
            std::cout << "Generating data for years " << config.start_year << " to " << config.end_year << "..." << '\n'; //
    
            LogGenerator generator(config.items_per_day, json_configs_opt->activities, json_configs_opt->remarks); //
            int files_generated = 0; //
    
            const std::string master_dir_name = "Date"; //
            try {
                if (!std::filesystem::exists(master_dir_name)) { //
                    std::filesystem::create_directory(master_dir_name); //
                    std::cout << "Created master directory: '" << master_dir_name << "'\n"; //
                }
            } catch(const std::filesystem::filesystem_error& e) {
                std::cerr << Utils::ConsoleColors::red << "Error creating master directory '" << master_dir_name << "'. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n'; //
                return 1; //
            }
            
            for (int year = config.start_year; year <= config.end_year; ++year) { //
                std::filesystem::path year_dir_path = std::filesystem::path(master_dir_name) / std::to_string(year); //
                
                try {
                    if (!std::filesystem::exists(year_dir_path)) { //
                        std::filesystem::create_directory(year_dir_path); //
                        std::cout << "Created directory: '" << year_dir_path.string() << "'\n"; //
                    }
                } catch(const std::filesystem::filesystem_error& e) {
                    std::cerr << Utils::ConsoleColors::red << "Error creating directory '" << year_dir_path.string() << "'. Detail: " << e.what() << Utils::ConsoleColors::reset << '\n'; //
                    return 1; //
                }
    
                for (int month = 1; month <= 12; ++month) { //
                    // MODIFIED: 使用 std::format 创建文件名，更简洁高效
                    std::string filename = std::format("{}_{:02}.txt", year, month);

                    std::filesystem::path full_path = year_dir_path / filename; //
    
                    std::ofstream outFile(full_path); //
                    if (!outFile.is_open()) { //
                        std::cerr << Utils::ConsoleColors::red << "Error: Could not open file '" << full_path.string() << "' for writing." << Utils::ConsoleColors::reset << '\n'; //
                        continue; //
                    }
    
                    int days_in_month = Utils::get_days_in_month(year, month); //
                    generator.generate_for_month(outFile, month, days_in_month); //
                    outFile.close(); //
                    files_generated++; //
                }
            }
    
            auto end_time = std::chrono::high_resolution_clock::now(); //
            report_completion(config, files_generated, start_time, end_time); //
            
            return 0; //
        }
    
    private:
        void report_completion(const Config& config,
                               int files_generated,
                               const std::chrono::high_resolution_clock::time_point& start, 
                               const std::chrono::high_resolution_clock::time_point& end) {
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); //
            auto duration_s = std::chrono::duration<double>(end - start); //
    
            // MODIFIED: 使用 std::format 统一输出，代码更整洁
            std::cout << std::format("{}\nData generation complete.{} {} monthly log files created for years {}-{}.\n",
                                     Utils::ConsoleColors::green,
                                     Utils::ConsoleColors::reset,
                                     files_generated,
                                     config.start_year,
                                     config.end_year);
            std::cout << std::format("Total generation time: {} ms ({:.3f} s).\n",
                                     duration_ms.count(),
                                     duration_s.count());
        }
    };

// --- 5. Main Entry Point ---
int main(int argc, char* argv[]) {
    Application app; //
    return app.run(argc, argv); //
}