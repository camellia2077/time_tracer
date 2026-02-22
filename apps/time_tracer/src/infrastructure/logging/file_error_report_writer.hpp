// infrastructure/logging/file_error_report_writer.hpp
#ifndef INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_
#define INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_

#include <filesystem>

#include "domain/ports/diagnostics.hpp"

namespace infrastructure::logging {

class FileErrorReportWriter final
    : public time_tracer::domain::ports::IErrorReportWriter {
 public:
  explicit FileErrorReportWriter(std::filesystem::path file_path);
  FileErrorReportWriter(std::filesystem::path run_file_path,
                        std::filesystem::path latest_file_path);

  auto Append(std::string_view report_content) -> bool override;
  [[nodiscard]] auto DestinationLabel() const -> std::string override;

 private:
  std::filesystem::path run_file_path_;
  std::filesystem::path latest_file_path_;
};

}  // namespace infrastructure::logging

#endif  // INFRASTRUCTURE_LOGGING_FILE_ERROR_REPORT_WRITER_H_
