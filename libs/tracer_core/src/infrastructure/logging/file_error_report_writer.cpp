// infrastructure/logging/file_error_report_writer.cpp
#include "infrastructure/logging/file_error_report_writer.hpp"

#include <fstream>
#include <system_error>
#include <utility>

namespace infrastructure::logging {

namespace {

auto EnsureParentDirectory(const std::filesystem::path& file_path) -> bool {
  std::error_code error_code;
  std::filesystem::create_directories(file_path.parent_path(), error_code);
  return !error_code;
}

auto AppendToFile(const std::filesystem::path& file_path,
                  std::string_view content) -> bool {
  std::ofstream stream(file_path, std::ios::app);
  if (!stream.is_open()) {
    return false;
  }
  stream << content;
  return stream.good();
}

auto TruncateFile(const std::filesystem::path& file_path) -> bool {
  std::ofstream stream(file_path, std::ios::trunc);
  return stream.is_open() && stream.good();
}

auto RemoveFileIfExists(const std::filesystem::path& file_path) -> void {
  if (file_path.empty()) {
    return;
  }

  std::error_code error_code;
  std::filesystem::remove(file_path, error_code);
}

}  // namespace

FileErrorReportWriter::FileErrorReportWriter(std::filesystem::path file_path)
    : run_file_path_(std::move(file_path)) {}

FileErrorReportWriter::FileErrorReportWriter(
    std::filesystem::path run_file_path, std::filesystem::path latest_file_path)
    : run_file_path_(std::move(run_file_path)),
      latest_file_path_(std::move(latest_file_path)) {
  RemoveFileIfExists(latest_file_path_);
}

auto FileErrorReportWriter::Append(std::string_view report_content) -> bool {
  if (!initialized_) {
    if (!run_file_path_.empty()) {
      if (!EnsureParentDirectory(run_file_path_)) {
        return false;
      }
      if (!TruncateFile(run_file_path_)) {
        return false;
      }
    }

    if (!latest_file_path_.empty()) {
      if (!EnsureParentDirectory(latest_file_path_)) {
        return false;
      }
      if (!TruncateFile(latest_file_path_)) {
        return false;
      }
    }

    initialized_ = true;
  }

  if (!run_file_path_.empty() &&
      !AppendToFile(run_file_path_, report_content)) {
    return false;
  }

  if (!latest_file_path_.empty() &&
      !AppendToFile(latest_file_path_, report_content)) {
      return false;
  }

  return true;
}

auto FileErrorReportWriter::DestinationLabel() const -> std::string {
  return run_file_path_.string();
}

}  // namespace infrastructure::logging
