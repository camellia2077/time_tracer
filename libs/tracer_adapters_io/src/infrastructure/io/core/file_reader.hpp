// infrastructure/io/core/file_reader.hpp
#ifndef INFRASTRUCTURE_IO_CORE_FILE_READER_H_
#define INFRASTRUCTURE_IO_CORE_FILE_READER_H_

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

/**
 * @brief 负责文件读取操作的静态工具类。
 */
class FileReader {
 public:
  /**
   * @brief 读取指定文件的所有原始字节。
   * @param path 文件路径。
   * @return 原始文件字节。
   * @throws std::runtime_error 如果文件不存在或无法打开。
   */
  static auto ReadBytes(const std::filesystem::path& path)
      -> std::vector<std::uint8_t>;

  /**
   * @brief 读取指定文件并返回规范化后的 UTF-8 文本。
   * @param path 文件路径。
   * @return 去 BOM + 统一 LF 后的 UTF-8 文本。
   * @throws std::runtime_error 如果文件不存在或无法打开。
   */
  static auto ReadCanonicalText(const std::filesystem::path& path)
      -> std::string;
};

#endif  // INFRASTRUCTURE_IO_CORE_FILE_READER_H_
