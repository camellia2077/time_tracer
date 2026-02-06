// importer/data_importer.cpp
#include "importer/data_importer.hpp"

#include <iomanip>
#include <iostream>

#include "common/ansi_colors.hpp"
#include "importer/import_service.hpp"

namespace {
// 纯 UI 逻辑：格式化打印报告
void PrintReport(const ImportStats& stats, const std::string& title) {
  double total_time = stats.parsing_duration_s + stats.db_insertion_duration_s;
  std::cout << "\n--- " << title << " Report ---" << std::endl;

  namespace colors = time_tracer::common::colors;
  if (!stats.db_open_success) {
    std::cerr << colors::kRed << "[Fatal] DB Error: "
              << (stats.error_message.empty() ? "Unknown" : stats.error_message)
              << colors::kReset << std::endl;
    return;
  }

  if (!stats.transaction_success) {
    std::cerr << colors::kRed
              << "[Fatal] Transaction Failed: " << stats.error_message
              << colors::kReset << std::endl;
    return;
  }

  if (stats.failed_files.empty()) {
    std::cout << colors::kGreen << "[Success] Processed " << stats.successful_files
              << " items." << colors::kReset << std::endl;
  } else {
    std::cout << colors::kYellow << "[Partial] Success: " << stats.successful_files
              << ", Failed: " << stats.failed_files.size() << colors::kReset
              << std::endl;
    for (const auto& failed_file : stats.failed_files) {
      std::cerr << "  Failed: " << failed_file << std::endl;
    }
  }

  std::cout << std::fixed << std::setprecision(3);
  std::cout << "Timing: Parse=" << stats.parsing_duration_s
            << "s, Insert=" << stats.db_insertion_duration_s
            << "s, Total=" << total_time << "s\n";
}
}  // namespace

// ---------------------------------------------------------
// Facade Implementation
// ---------------------------------------------------------

void HandleProcessMemoryData(

    const std::string& db_name,
    const std::map<std::string, std::vector<DailyLog>>& data) {
  std::cout << "Task: Memory Import..." << std::endl;

  ImportService service(db_name);
  ImportStats stats = service.ImportFromMemory(data);

  PrintReport(stats, "Memory Import");
}