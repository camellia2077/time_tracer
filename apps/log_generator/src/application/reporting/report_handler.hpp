// application/reporting/report_handler.hpp
#ifndef APPLICATION_REPORTING_REPORT_HANDLER_H_
#define APPLICATION_REPORTING_REPORT_HANDLER_H_

#include <chrono>

#include "common/config_types.hpp"
#include "utils/performance_reporter.hpp"

namespace App {

class ReportHandler {
 public:
  ReportHandler();
  PerformanceReporter& get_reporter();
  void finish(const Config& config, int files_generated);

 private:
  PerformanceReporter reporter_;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
};

}  // namespace App

#endif  // APPLICATION_REPORTING_REPORT_HANDLER_H_
