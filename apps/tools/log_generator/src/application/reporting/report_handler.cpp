// application/reporting/report_handler.cpp
#include "application/reporting/report_handler.hpp"

namespace App {

ReportHandler::ReportHandler() {
  start_time_ = std::chrono::high_resolution_clock::now();
}

auto ReportHandler::get_reporter() -> PerformanceReporter& {
  return reporter_;
}

void ReportHandler::finish(const Config& config, int files_generated) {
  auto end_time = std::chrono::high_resolution_clock::now();
  reporter_.report(config, files_generated, end_time - start_time_);
}

}  // namespace App
