// application/workflow/workflow_handler.cpp
#include "application/workflow/workflow_handler.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "application/workflow/workflow_monthly_average_stats.hpp"
#include "utils/utils.hpp"

namespace {
constexpr unsigned int kDefaultMaxConcurrent = 4;
constexpr size_t kBytesPerKibibyte = 1024U;
constexpr size_t kInitialBufferSize = kBytesPerKibibyte * kBytesPerKibibyte;
constexpr int kMonthsPerYear = 12;

struct TaskResult {
  int generated_files = 0;
  std::vector<App::workflow_stats::MonthlyAverageStat> monthly_stats;
};
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

  const bool kEnableMonthlyAverage =
      context.config.enable_monthly_average_report;
  const std::unordered_set<std::string> kWakeKeywords(context.wake_keywords
                                                          .begin(),
                                                      context.wake_keywords
                                                          .end());

  std::vector<std::future<TaskResult>> active_tasks;
  int total_files_generated = 0;
  std::vector<workflow_stats::MonthlyAverageStat> all_monthly_stats;

  for (int year = context.config.start_year; year <= context.config.end_year;
       ++year) {
    auto completed_tasks = std::ranges::remove_if(
        active_tasks, [&total_files_generated, &all_monthly_stats,
                       kEnableMonthlyAverage](
                          std::future<TaskResult>& task_future) -> bool {
          if (task_future.wait_for(std::chrono::seconds(0)) ==
              std::future_status::ready) {
            TaskResult result = task_future.get();
            total_files_generated += result.generated_files;
            if (kEnableMonthlyAverage) {
              all_monthly_stats.insert(all_monthly_stats.end(),
                                       result.monthly_stats.begin(),
                                       result.monthly_stats.end());
            }
            return true;
          }
          return false;
        });
    active_tasks.erase(completed_tasks.begin(), completed_tasks.end());

    if (active_tasks.size() >= max_concurrent) {
      TaskResult result = active_tasks.front().get();
      total_files_generated += result.generated_files;
      if (kEnableMonthlyAverage) {
        all_monthly_stats.insert(all_monthly_stats.end(),
                                 result.monthly_stats.begin(),
                                 result.monthly_stats.end());
      }
      active_tasks.erase(active_tasks.begin());
    }

    active_tasks.push_back(std::async(
        std::launch::async,
        [=, this, &context, &reporter, wake_keywords = kWakeKeywords]()
            -> TaskResult {
          auto generator = generator_factory_.create(context);

          std::string buffer;
          buffer.reserve(kInitialBufferSize);

          TaskResult result;

          for (int month = 1; month <= kMonthsPerYear; ++month) {
            std::string filename = std::format("{}-{:02}.txt", year, month);
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

            if (kEnableMonthlyAverage) {
              result.monthly_stats.push_back(
                  workflow_stats::BuildMonthlyAverageStat(
                      year, month, buffer, wake_keywords));
            }

            auto io_start = std::chrono::high_resolution_clock::now();
            if (file_system_.write_log_file(full_path, buffer)) {
              result.generated_files++;
            }
            reporter.add_io_time(std::chrono::high_resolution_clock::now() -
                                 io_start);
          }

          return result;
        }));
  }

  for (auto& task : active_tasks) {
    TaskResult result = task.get();
    total_files_generated += result.generated_files;
    if (kEnableMonthlyAverage) {
      all_monthly_stats.insert(all_monthly_stats.end(),
                               result.monthly_stats.begin(),
                               result.monthly_stats.end());
    }
  }

  if (kEnableMonthlyAverage) {
    workflow_stats::PrintMonthlyAverageReport(std::move(all_monthly_stats));
  }

  return total_files_generated;
}
}  // namespace App
