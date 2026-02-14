// infrastructure/logging/file_error_report_writer.cpp
#include "infrastructure/logging/file_error_report_writer.hpp"

#include <fstream>
#include <system_error>
#include <utility>

namespace infrastructure::logging {

FileErrorReportWriter::FileErrorReportWriter(std::filesystem::path file_path)
    : file_path_(std::move(file_path)) {}

auto FileErrorReportWriter::Append(std::string_view report_content) -> bool {
  std::error_code error_code;
  std::filesystem::create_directories(file_path_.parent_path(), error_code);
  if (error_code) {
    return false;
  }

  std::ofstream stream(file_path_, std::ios::app);
  if (!stream.is_open()) {
    return false;
  }
  stream << report_content;
  return stream.good();
}

auto FileErrorReportWriter::DestinationLabel() const -> std::string {
  return file_path_.string();
}

}  // namespace infrastructure::logging
