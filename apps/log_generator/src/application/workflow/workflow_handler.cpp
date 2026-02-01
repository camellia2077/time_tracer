// application/workflow/workflow_handler.cpp
#include "application/workflow/workflow_handler.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "utils/utils.hpp"

namespace App {

WorkflowHandler::WorkflowHandler(FileSystem& file_system,
                                 ILogGeneratorFactory& generator_factory)
    : file_system_(file_system), generator_factory_(generator_factory) {}

auto WorkflowHandler::run(const Core::AppContext& context,
                          ReportHandler& report_handler) -> int {
  const std::string kMasterDirName = "dates";

  if (!file_system_.setup_directories(kMasterDirName, context.config.start_year,
                                      context.config.end_year)) {
    return -1;
  }

  auto& reporter = report_handler.get_reporter();

  unsigned int max_concurrent = std::thread::hardware_concurrency();
  if (max_concurrent == 0) {
    max_concurrent = 4;
  }

  std::cout << "Starting generation with " << max_concurrent
            << " concurrent threads...\n";

  std::vector<std::future<int>> active_tasks;
  int total_files_generated = 0;

  for (int year = context.config.start_year; year <= context.config.end_year;
       ++year) {
    auto it = std::remove_if(
        active_tasks.begin(),
        active_tasks.end(),
        [&total_files_generated](std::future<int>& f) -> bool {
          if (f.wait_for(std::chrono::seconds(0)) ==
              std::future_status::ready) {
            total_files_generated += f.get();
            return true;
          }
          return false;
        });
    active_tasks.erase(it, active_tasks.end());

    if (active_tasks.size() >= max_concurrent) {
      total_files_generated += active_tasks.front().get();
      active_tasks.erase(active_tasks.begin());
    }

    active_tasks.push_back(std::async(
        std::launch::async,
        [=, this, &context, &reporter, &kMasterDirName]() -> int {
          auto generator = generator_factory_.create(context);

          std::string buffer;
          buffer.reserve(1024 * 1024);

          int local_files_generated = 0;

          for (int month = 1; month <= 12; ++month) {
            std::string filename = std::format("{}_{:02}.txt", year, month);
            std::filesystem::path full_path =
                std::filesystem::path(kMasterDirName) / std::to_string(year) /
                filename;

            auto gen_start = std::chrono::high_resolution_clock::now();
            generator->generate_for_month(
                year, month, Utils::get_days_in_month(year, month), buffer);
            reporter.add_generation_time(
                std::chrono::high_resolution_clock::now() - gen_start);

            auto io_start = std::chrono::high_resolution_clock::now();
            if (file_system_.write_log_file(full_path, buffer)) {
              local_files_generated++;
            }
            reporter.add_io_time(std::chrono::high_resolution_clock::now() -
                                 io_start);
          }

          return local_files_generated;
        }));
  }

  for (auto& task : active_tasks) {
    total_files_generated += task.get();
  }

  return total_files_generated;
}
}  // namespace App
