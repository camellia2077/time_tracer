// application/insights/insights_handler.cpp
#include "application/insights/insights_handler.hpp"

namespace App {

InsightsHandler::InsightsHandler() {
  start_time_ = std::chrono::high_resolution_clock::now();
}

auto InsightsHandler::get_reporter() -> PerformanceReporter& {
  return reporter_;
}

void InsightsHandler::finish(const Config& config, int files_generated) {
  auto end_time = std::chrono::high_resolution_clock::now();
  reporter_.report(config, files_generated, end_time - start_time_);
}

}  // namespace App
