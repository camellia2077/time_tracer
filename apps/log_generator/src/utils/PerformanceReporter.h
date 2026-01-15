#ifndef UTILS_PERFORMANCEREPORTER_H
#define UTILS_PERFORMANCEREPORTER_H

#include "common/ConfigTypes.hpp"
#include <chrono>
#include <mutex> // [新增]

class PerformanceReporter {
public:
    PerformanceReporter();

    void add_generation_time(const std::chrono::nanoseconds& duration);
    void add_io_time(const std::chrono::nanoseconds& duration);

    void report(const Config& config, int files_generated, const std::chrono::nanoseconds& total_duration) const;

private:
    std::chrono::nanoseconds total_generation_duration_;
    std::chrono::nanoseconds total_io_duration_;
    mutable std::mutex mutex_; // [新增] 保护累加操作
};

#endif // UTILS_PERFORMANCEREPORTER_H