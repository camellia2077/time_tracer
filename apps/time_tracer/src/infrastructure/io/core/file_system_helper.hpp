// infrastructure/io/core/file_system_helper.hpp
#ifndef INFRASTRUCTURE_IO_CORE_FILE_SYSTEM_HELPER_H_
#define INFRASTRUCTURE_IO_CORE_FILE_SYSTEM_HELPER_H_

#include <filesystem>

/**
 * @brief 文件系统操作的封装助手类。
 * @details 提供了对 std::filesystem 常见操作的包装，
 * 旨在统一异常处理并解耦上层业务逻辑与底层文件系统 API。
 */
class FileSystemHelper {
 public:
  /**
   * @brief 检查路径是否存在。
   */
  [[nodiscard]] static auto Exists(const std::filesystem::path& path) -> bool;

  /**
   * @brief 检查路径是否为普通文件。
   */
  [[nodiscard]] static auto IsRegularFile(const std::filesystem::path& path)
      -> bool;

  /**
   * @brief 检查路径是否为目录。
   */
  [[nodiscard]] static auto IsDirectory(const std::filesystem::path& path)
      -> bool;

  /**
   * @brief 递归创建目录。
   * @details 类似于 mkdir -p。如果目录已存在，则不执行任何操作。
   * @param path 要创建的目录路径。
   * @throws std::runtime_error 如果创建失败。
   */
  static auto CreateDirectories(const std::filesystem::path& path) -> void;
};

#endif  // INFRASTRUCTURE_IO_CORE_FILE_SYSTEM_HELPER_H_
