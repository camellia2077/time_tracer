// importer/data_importer.cpp
#include "importer/data_importer.hpp"
#include "importer/import_service.hpp" 
#include "common/ansi_colors.hpp" 
#include <iostream>
#include <iomanip>

namespace {
// 纯 UI 逻辑：格式化打印报告
void print_report(const ImportStats& stats, const std::string& title) {
    double total_time = stats.parsing_duration_s + stats.db_insertion_duration_s;
    std::cout << "\n--- " << title << " Report ---" << std::endl;

    if (!stats.db_open_success) {
        std::cerr << RED_COLOR << "[Fatal] DB Error: " << (stats.error_message.empty() ? "Unknown" : stats.error_message) << RESET_COLOR << std::endl;
        return;
    }

    if (!stats.transaction_success) {
        std::cerr << RED_COLOR << "[Fatal] Transaction Failed: " << stats.error_message << RESET_COLOR << std::endl;
        return;
    }

    if (stats.failed_files.empty()) {
        std::cout << GREEN_COLOR << "[Success] Processed " << stats.successful_files << " items." << RESET_COLOR << std::endl;
    } else {
        std::cout << YELLOW_COLOR << "[Partial] Success: " << stats.successful_files << ", Failed: " << stats.failed_files.size() << RESET_COLOR << std::endl;
        for (const auto& f : stats.failed_files) std::cerr << "  Failed: " << f << std::endl;
    }

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Timing: Parse=" << stats.parsing_duration_s << "s, Insert=" << stats.db_insertion_duration_s << "s, Total=" << total_time << "s\n";
}
}

// ---------------------------------------------------------
// Facade Implementation
// ---------------------------------------------------------

void handle_process_memory_data(const std::string& db_name, const std::map<std::string, std::vector<DailyLog>>& data) {
    std::cout << "Task: Memory Import..." << std::endl;
    
    ImportService service(db_name);
    ImportStats stats = service.import_from_memory(data);
    
    print_report(stats, "Memory Import");
}