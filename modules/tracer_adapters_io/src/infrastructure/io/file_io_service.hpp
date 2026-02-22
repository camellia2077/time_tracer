// infrastructure/io/file_io_service.hpp
#ifndef INFRASTRUCTURE_IO_FILE_IO_SERVICE_H_
#define INFRASTRUCTURE_IO_FILE_IO_SERVICE_H_

#include <filesystem>
#include <vector>

class FileIoService {
 public:
  FileIoService() = default;

  static auto FindLogFiles(const std::filesystem::path& root_path)
      -> std::vector<std::filesystem::path>;

  static void PrepareOutputDirectories(
      const std::filesystem::path& output_root);
};

#endif  // INFRASTRUCTURE_IO_FILE_IO_SERVICE_H_
