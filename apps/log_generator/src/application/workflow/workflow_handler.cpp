// application/workflow/workflow_handler.cpp
#include "application/workflow/workflow_handler.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "utils/utils.hpp"

namespace {
constexpr unsigned int kDefaultMaxConcurrent = 4;
constexpr size_t kBytesPerKibibyte = 1024U;
constexpr size_t kInitialBufferSize = kBytesPerKibibyte * kBytesPerKibibyte;
constexpr int kMonthsPerYear = 12;
}  // namespace

namespace App {

WorkflowHandler::WorkflowHandler(FileSystem& file_system,
                                 ILogGeneratorFactory& generator_factory)
    : file_system_(file_system), generator_factory_(generator_factory) {}

auto WorkflowHandler::run(const Core::AppContext& context,
                          ReportHandler& report_handler) -> int {
  const std::filesystem::path kOutputRoot = context.config.output_directory;

  const YearRange kYearRange{.start_year = context.config.start_year,
                             .end_year = context.config.end_year};
  if (!file_system_.setup_directories(kOutputRoot.string(), kYearRange)) {
    return -1;
  }

  auto& reporter = report_handler.get_reporter();

  unsigned int max_concurrent = std::thread::hardware_concurrency();
  if (max_concurrent == 0) {
    max_concurrent = kDefaultMaxConcurrent;
  }

  std::cout << "Starting generation with " << max_concurrent
            << " concurrent threads...\n";

  std::vector<std::future<int>> active_tasks;
  int total_files_generated = 0;

  for (int year = context.config.start_year; year <= context.config.end_year;
       ++year) {
    auto completed_tasks = std::ranges::remove_if(
        active_tasks,
        [&total_files_generated](std::future<int>& task_future) -> bool {
          if (task_future.wait_for(std::chrono::seconds(0)) ==
              std::future_status::ready) {
            total_files_generated += task_future.get();
            return true;
          }
          return false;
        });
    active_tasks.erase(completed_tasks.begin(), active_tasks.end());

    if (active_tasks.size() >= max_concurrent) {
      total_files_generated += active_tasks.front().get();
      active_tasks.erase(active_tasks.begin());
    }

    active_tasks.push_back(
        std::async(std::launch::async, [=, this, &context, &reporter]() -> int {
          auto generator = generator_factory_.create(context);

          std::string buffer;
          buffer.reserve(kInitialBufferSize);

          int local_files_generated = 0;

          for (int month = 1; month <= kMonthsPerYear; ++month) {
            std::string filename = std::format("{}_{:02}.txt", year, month);
            std::filesystem::path full_path =
                kOutputRoot / std::to_string(year) / filename;

            auto gen_start = std::chrono::high_resolution_clock::now();
            const Utils::YearMonth kYearMonth{.year = year, .month = month};
            const int kDaysInMonth = Utils::get_days_in_month(kYearMonth);
            const MonthContext kMonthContext{
                .year = year, .month = month, .days_in_month = kDaysInMonth};
            generator->generate_for_month(kMonthContext, buffer);
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
