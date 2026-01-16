// core/reporting/ReportHandler.cpp
#include "ReportHandler.hpp"

namespace Core {

    ReportHandler::ReportHandler() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    PerformanceReporter& ReportHandler::get_reporter() {
        return reporter_;
    }

    void ReportHandler::finish(const Config& config, int files_generated) {
        auto end_time = std::chrono::high_resolution_clock::now();
        reporter_.report(config, files_generated, end_time - start_time_);
    }

}