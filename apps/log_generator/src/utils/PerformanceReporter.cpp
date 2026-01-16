// utils/PerformanceReporter.cpp
#include "utils/PerformanceReporter.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <format>

PerformanceReporter::PerformanceReporter()
    : total_generation_duration_(0), total_io_duration_(0) {}

void PerformanceReporter::add_generation_time(const std::chrono::nanoseconds& duration) {
    std::lock_guard<std::mutex> lock(mutex_); 
    total_generation_duration_ += duration;
}

void PerformanceReporter::add_io_time(const std::chrono::nanoseconds& duration) {
    std::lock_guard<std::mutex> lock(mutex_); 
    total_io_duration_ += duration;
}

void PerformanceReporter::report(const Config& config, int files_generated, const std::chrono::nanoseconds& total_duration) const {
    auto total_s = std::chrono::duration<double>(total_duration);
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);

    auto gen_s = std::chrono::duration<double>(total_generation_duration_);
    auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_generation_duration_);

    auto io_s = std::chrono::duration<double>(total_io_duration_);
    auto io_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_io_duration_);

    std::cout << std::format("{}\nData generation complete.{} {} monthly log files created for years {}-{}.\n",
                             GREEN_COLOR,
                             RESET_COLOR,
                             files_generated,
                             config.start_year,
                             config.end_year);
    
    std::cout << "--------------------------------------------------\n";
    // 使用更准确的时间统计术语
    std::cout << std::format("Total Wall Time:      {:.2f} s ({}ms)\n", total_s.count(), total_ms.count());
    std::cout << "--------------------------------------------------\n";
    // 增加 Cumulative 前缀，并解释这是所有线程之和
    std::cout << std::format("Cumulative Gen Time:  {:.2f} s ({}ms) [Sum of all threads]\n", gen_s.count(), gen_ms.count());
    std::cout << std::format("Cumulative IO Time:   {:.2f} s ({}ms) [Sum of all threads]\n", io_s.count(), io_ms.count());
    std::cout << "--------------------------------------------------\n";
}