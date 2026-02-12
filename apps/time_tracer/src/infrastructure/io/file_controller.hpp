// infrastructure/io/file_controller.hpp
#ifndef INFRASTRUCTURE_IO_FILE_CONTROLLER_H_
#define INFRASTRUCTURE_IO_FILE_CONTROLLER_H_

#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief 文件系统外观类。
 * @details 提供底层的文件查找和目录准备服务。
 * 此时它已不再负责加载配置，成为了纯粹的 IO 辅助组件。
 */
class FileController {
 public:
  /**
   * @brief 默认构造函数。
   */
  FileController() = default;

  /**
   * @brief 递归查找指定目录下的日志文件(.txt)。
   */
  static auto FindLogFiles(const std::filesystem::path& root_path)
      -> std::vector<std::filesystem::path>;

  /**
   * @brief 准备输出环境（创建必要的目录）。
   * @details 将环境初始化的 IO 操作封装在此。
   * @param output_root 根输出目录。
   */
  static void PrepareOutputDirectories(
      const std::filesystem::path& output_root);
};

#endif  // INFRASTRUCTURE_IO_FILE_CONTROLLER_H_
