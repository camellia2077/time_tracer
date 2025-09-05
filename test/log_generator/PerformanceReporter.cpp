#include "PerformanceReporter.h"
#include "Utils.h" // 需要用到颜色输出
#include <iostream>
#include <format>

PerformanceReporter::PerformanceReporter()
    : total_generation_duration_(0), total_io_duration_(0) {}

void PerformanceReporter::add_generation_time(const std::chrono::nanoseconds& duration) {
    total_generation_duration_ += duration;
}

void PerformanceReporter::add_io_time(const std::chrono::nanoseconds& duration) {
    total_io_duration_ += duration;
}

void PerformanceReporter::report(const Config& config, int files_generated, const std::chrono::nanoseconds& total_duration) const {
    // 将所有时长转换为秒（double）和毫秒（long long）
    auto total_s = std::chrono::duration<double>(total_duration);
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);

    auto gen_s = std::chrono::duration<double>(total_generation_duration_);
    auto gen_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_generation_duration_);

    auto io_s = std::chrono::duration<double>(total_io_duration_);
    auto io_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_io_duration_);

    std::cout << std::format("{}\nData generation complete.{} {} monthly log files created for years {}-{}.\n",
                             Utils::ConsoleColors::green,
                             Utils::ConsoleColors::reset,
                             files_generated,
                             config.start_year,
                             config.end_year);
    
    std::cout << "---------------------------\n";
    std::cout << std::format("total time:    {:.2f} s ({}ms)\n", total_s.count(), total_ms.count());
    std::cout << std::format("text generate: {:.2f} s ({}ms)\n", gen_s.count(), gen_ms.count());
    std::cout << std::format("io:            {:.2f} s ({}ms)\n", io_s.count(), io_ms.count());
    std::cout << "---------------------------\n";
}