// application/ports/file_system.hpp
#ifndef APPLICATION_PORTS_FILE_SYSTEM_H_
#define APPLICATION_PORTS_FILE_SYSTEM_H_

#include <filesystem>
#include <optional>
#include <string>

class FileSystem {
 public:
  virtual ~FileSystem() = default;
  virtual bool setup_directories(const std::string& master_dir, int start_year,
                                 int end_year) = 0;
  virtual bool write_log_file(const std::filesystem::path& file_path,
                              const std::string& content) = 0;
  virtual std::optional<std::string> read_file(
      const std::filesystem::path& file_path) = 0;
};

#endif  // APPLICATION_PORTS_FILE_SYSTEM_H_
