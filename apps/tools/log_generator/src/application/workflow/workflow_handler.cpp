// application/workflow/workflow_handler.cpp
#include "application/workflow/workflow_handler.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <format>
#include <future>
#include <iostream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "utils/utils.hpp"

namespace {
constexpr unsigned int kDefaultMaxConcurrent = 4;
constexpr size_t kBytesPerKibibyte = 1024U;
constexpr size_t kInitialBufferSize = kBytesPerKibibyte * kBytesPerKibibyte;
constexpr int kMonthsPerYear = 12;
constexpr int kMinutesPerDay = 24 * 60;

struct ParsedEventForStats {
  int minute_of_day = 0;
  std::string description;
};

struct ParsedDayForStats {
  std::optional<int> getup_minute;
  bool is_continuation = false;
  std::vector<ParsedEventForStats> events;
};

struct MonthlyAverageStat {
  int year = 0;
  int month = 0;
  int day_count = 0;
  double average_minutes_per_day = 0.0;
};

struct TaskResult {
  int generated_files = 0;
  std::vector<MonthlyAverageStat> monthly_stats;
};

auto IsAsciiDigit(char value) -> bool {
  return (value >= '0') && (value <= '9');
}

auto TrimAscii(std::string_view input) -> std::string_view {
  size_t begin = 0;
  while (begin < input.size() &&
         std::isspace(static_cast<unsigned char>(input[begin])) != 0) {
    ++begin;
  }

  size_t end = input.size();
  while (end > begin &&
         std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
    --end;
  }

  return input.substr(begin, end - begin);
}

auto IsDayMarker(std::string_view line) -> bool {
  return line.size() == 4 &&
         std::ranges::all_of(line, [](char value) -> bool {
           return IsAsciiDigit(value);
         });
}

auto IsEventLine(std::string_view line) -> bool {
  if (line.size() < 5) {
    return false;
  }
  return std::ranges::all_of(line.substr(0, 4), [](char value) -> bool {
    return IsAsciiDigit(value);
  });
}

auto ParseMinute(std::string_view hhmm) -> int {
  int hour = std::stoi(std::string(hhmm.substr(0, 2)));
  int minute = std::stoi(std::string(hhmm.substr(2, 2)));
  return (hour * 60) + minute;
}

auto ExtractDescription(std::string_view event_tail) -> std::string {
  size_t delimiter_pos = std::string_view::npos;
  constexpr std::array<std::string_view, 3> kDelimiters = {"//", "#", ";"};
  for (std::string_view delimiter : kDelimiters) {
    size_t pos = event_tail.find(delimiter);
    if (pos != std::string_view::npos &&
        (delimiter_pos == std::string_view::npos || pos < delimiter_pos)) {
      delimiter_pos = pos;
    }
  }

  std::string_view description =
      (delimiter_pos == std::string_view::npos)
          ? event_tail
          : event_tail.substr(0, delimiter_pos);
  return std::string(TrimAscii(description));
}

auto DiffWrapMinutes(int start_minute, int end_minute) -> int {
  if (end_minute < start_minute) {
    end_minute += kMinutesPerDay;
  }
  return end_minute - start_minute;
}

auto ParseMonthDaysForStats(
    const std::string& month_content,
    const std::unordered_set<std::string>& wake_keywords)
    -> std::vector<ParsedDayForStats> {
  std::vector<ParsedDayForStats> days;
  std::istringstream stream(month_content);
  std::string line;
  ParsedDayForStats* current_day = nullptr;

  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    std::string_view view = TrimAscii(line);
    if (view.empty()) {
      continue;
    }

    if ((view[0] == 'y') || (view[0] == 'm')) {
      continue;
    }

    if (IsDayMarker(view)) {
      days.emplace_back();
      current_day = &days.back();
      continue;
    }

    if (current_day == nullptr || !IsEventLine(view)) {
      continue;
    }

    const int minute_of_day = ParseMinute(view.substr(0, 4));
    std::string description = ExtractDescription(view.substr(4));
    const bool is_wake = wake_keywords.contains(description);

    if (is_wake) {
      if (!current_day->getup_minute.has_value()) {
        current_day->getup_minute = minute_of_day;
      }
    } else if (!current_day->getup_minute.has_value() &&
               current_day->events.empty()) {
      // Match parser behavior: first non-wake event without getup marks
      // continuation.
      current_day->is_continuation = true;
    }

    current_day->events.push_back(
        ParsedEventForStats{.minute_of_day = minute_of_day,
                            .description = std::move(description)});
  }

  return days;
}

auto CalculateMonthAverageFromDays(
    const std::vector<ParsedDayForStats>& days,
    const std::unordered_set<std::string>& wake_keywords) -> double {
  if (days.empty()) {
    return 0.0;
  }

  long long total_minutes = 0;
  std::optional<int> previous_day_last_minute;

  for (const auto& day : days) {
    long long day_minutes = 0;

    if (previous_day_last_minute.has_value() && day.getup_minute.has_value() &&
        !day.is_continuation) {
      day_minutes +=
          DiffWrapMinutes(*previous_day_last_minute, *day.getup_minute);
    }

    std::optional<int> current_start_minute;
    if (day.is_continuation && previous_day_last_minute.has_value()) {
      current_start_minute = *previous_day_last_minute;
    } else if (day.getup_minute.has_value()) {
      current_start_minute = *day.getup_minute;
    }

    for (const auto& event : day.events) {
      const bool is_wake = wake_keywords.contains(event.description);
      if (is_wake) {
        if (!current_start_minute.has_value()) {
          current_start_minute = event.minute_of_day;
        }
        continue;
      }

      if (!current_start_minute.has_value()) {
        continue;
      }

      day_minutes +=
          DiffWrapMinutes(*current_start_minute, event.minute_of_day);
      current_start_minute = event.minute_of_day;
    }

    if (!day.events.empty()) {
      previous_day_last_minute = day.events.back().minute_of_day;
    }

    total_minutes += day_minutes;
  }

  return static_cast<double>(total_minutes) / static_cast<double>(days.size());
}

auto BuildMonthlyAverageStat(
    int year, int month, const std::string& month_content,
    const std::unordered_set<std::string>& wake_keywords) -> MonthlyAverageStat {
  const auto days = ParseMonthDaysForStats(month_content, wake_keywords);
  return MonthlyAverageStat{
      .year = year,
      .month = month,
      .day_count = static_cast<int>(days.size()),
      .average_minutes_per_day =
          CalculateMonthAverageFromDays(days, wake_keywords)};
}

void PrintMonthlyAverageReport(std::vector<MonthlyAverageStat> stats) {
  if (stats.empty()) {
    return;
  }

  std::ranges::sort(stats, [](const MonthlyAverageStat& lhs,
                              const MonthlyAverageStat& rhs) -> bool {
    if (lhs.year != rhs.year) {
      return lhs.year < rhs.year;
    }
    return lhs.month < rhs.month;
  });

  std::cout << "\nMonthly Average Tracked Time (Generated Data):\n";
  std::cout << "--------------------------------------------------\n";
  for (const auto& stat : stats) {
    const double avg_hours = stat.average_minutes_per_day / 60.0;
    std::cout << std::format(
        "  {}-{:02}: {:6.2f} h/day ({:7.2f} min/day, {} days)\n", stat.year,
        stat.month, avg_hours, stat.average_minutes_per_day, stat.day_count);
  }
  std::cout << "--------------------------------------------------\n";
}
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
  std::vector<MonthlyAverageStat> all_monthly_stats;

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
              result.monthly_stats.push_back(BuildMonthlyAverageStat(
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
    PrintMonthlyAverageReport(std::move(all_monthly_stats));
  }

  return total_files_generated;
}
}  // namespace App
