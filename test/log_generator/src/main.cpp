#include "config/Config.h"
#include "generator/LogGenerator.h"
#include "utils/Utils.h"
#include "file_io/FileManager.h"
#include "utils/PerformanceReporter.h"
#include "cli/CommandLineParser.h"
#include "common/AnsiColors.hpp"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <format>

// --- Application Runner ---
class Application {
public:
    int run(int argc, char* argv[]) {
        Utils::setup_console();

        CommandLineParser parser(argc, argv);
        
        if (parser.version_requested()) {
            parser.print_version();
            return 0;
        }

        auto config_opt = parser.parse();
        if (!config_opt) {
            return 1;
        }
        Config config = *config_opt;

        std::filesystem::path exe_path = argv[0];
        std::filesystem::path exe_dir = exe_path.parent_path();
        std::filesystem::path config_file_path = exe_dir / "config" / "activities_config.json";
        
        auto json_configs_opt = ConfigLoader::load_json_configurations(config_file_path.string());
        
        if (!json_configs_opt) {
            std::cerr << RED_COLOR << "Exiting program due to configuration loading failure." << RESET_COLOR << std::endl;
            return 1;
        }

        auto total_start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Generating data for years " << config.start_year << " to " << config.end_year << "..." << '\n';

        LogGenerator generator(config.items_per_day, json_configs_opt->activities, json_configs_opt->remarks);
        FileManager file_manager;
        PerformanceReporter reporter;

        const std::string master_dir_name = "Date";
        if (!file_manager.setup_directories(master_dir_name, config.start_year, config.end_year)) {
            return 1;
        }

        int files_generated = 0;
        // [核心修改] 修正 for 循环中的笔误，使用 config.end_year
        for (int year = config.start_year; year <= config.end_year; ++year) {
            for (int month = 1; month <= 12; ++month) {
                std::string filename = std::format("{}_{:02}.txt", year, month);
                std::filesystem::path full_path = std::filesystem::path(master_dir_name) / std::to_string(year) / filename;

                auto gen_start = std::chrono::high_resolution_clock::now();
                int days_in_month = Utils::get_days_in_month(year, month);
                std::string month_log = generator.generate_for_month(year, month, days_in_month);
                auto gen_end = std::chrono::high_resolution_clock::now();
                reporter.add_generation_time(gen_end - gen_start);

                auto io_start = std::chrono::high_resolution_clock::now();
                if (!file_manager.write_log_file(full_path, month_log)) {
                    continue;
                }
                auto io_end = std::chrono::high_resolution_clock::now();
                reporter.add_io_time(io_end - io_start);

                files_generated++;
            }
        }

        auto total_end_time = std::chrono::high_resolution_clock::now();
        reporter.report(config, files_generated, total_end_time - total_start_time);
        
        return 0;
    }
};

// --- Main Entry Point ---
int main(int argc, char* argv[]) {
    Application app;
    return app.run(argc, argv);
}