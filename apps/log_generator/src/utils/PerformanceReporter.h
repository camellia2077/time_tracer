#pragma once
#include "config/Config.h"
#include <chrono>

class PerformanceReporter {
public:
    PerformanceReporter();

    // 累加生成时间
    void add_generation_time(const std::chrono::nanoseconds& duration);
    // 累加IO时间
    void add_io_time(const std::chrono::nanoseconds& duration);

    // 打印最终的性能报告
    void report(const Config& config, int files_generated, const std::chrono::nanoseconds& total_duration) const;

private:
    std::chrono::nanoseconds total_generation_duration_;
    std::chrono::nanoseconds total_io_duration_;
};