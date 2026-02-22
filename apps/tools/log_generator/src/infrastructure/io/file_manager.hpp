// infrastructure/io/file_manager.hpp
#ifndef INFRASTRUCTURE_IO_FILE_MANAGER_H_
#define INFRASTRUCTURE_IO_FILE_MANAGER_H_

#include <filesystem>
#include <optional>
#include <string>

#include "application/ports/file_system.hpp"

class FileManager : public FileSystem {
 public:
  bool setup_directories(const std::string& master_dir,
                         const YearRange& year_range) override;
  bool write_log_file(const std::filesystem::path& file_path,
                      const std::string& content) override;
  std::optional<std::string> read_file(
      const std::filesystem::path& file_path) override;
};

#endif  // INFRASTRUCTURE_IO_FILE_MANAGER_H_
