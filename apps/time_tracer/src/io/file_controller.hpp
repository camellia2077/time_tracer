// io/file_controller.hpp
#ifndef IO_FILE_CONTROLLER_H_
#define IO_FILE_CONTROLLER_H_

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
  static std::vector<std::filesystem::path> find_log_files(
      const std::filesystem::path& root_path);

  /**
   * @brief 准备输出环境（创建必要的目录）。
   * @details 将环境初始化的 IO 操作封装在此。
   * @param output_root 根输出目录。
   * @param export_root 导出文件目录。
   */
  static void prepare_output_directories(
      const std::filesystem::path& output_root,
      const std::filesystem::path& export_root);
};

#endif  // IO_FILE_CONTROLLER_H_