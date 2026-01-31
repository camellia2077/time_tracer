// io/core/file_reader.hpp
#ifndef IO_CORE_FILE_READER_H_
#define IO_CORE_FILE_READER_H_

#include <filesystem>
#include <string>

/**
 * @brief 负责文件读取操作的静态工具类。
 */
class FileReader {
 public:
  /**
   * @brief 读取指定文件的所有内容。
   * @param path 文件路径。
   * @return 文件内容字符串。
   * @throws std::runtime_error 如果文件不存在或无法打开。
   */
  static std::string read_content(const std::filesystem::path& path);
};

#endif  // IO_CORE_FILE_READER_H_