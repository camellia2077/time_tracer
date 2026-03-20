// infrastructure/io/core/file_writer.hpp
#ifndef INFRASTRUCTURE_IO_CORE_FILE_WRITER_H_
#define INFRASTRUCTURE_IO_CORE_FILE_WRITER_H_

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

/**
 * @brief 负责文件写入操作的静态工具类。
 */
class FileWriter {
 public:
  /**
   * @brief 将原始字节写入指定文件。
   * @param path 目标文件路径。
   * @param bytes 要写入的字节。
   * @throws std::runtime_error 如果文件无法打开或写入失败。
   */
  static void WriteBytes(const std::filesystem::path& path,
                         std::span<const std::uint8_t> bytes);

  /**
   * @brief 将文本按 canonical UTF-8 规则写入指定文件。
   * @param path 目标文件路径。
   * @param content 要写入的文本内容。
   * @throws std::runtime_error 如果文件无法打开、内容不是有效 UTF-8 或写入失败。
   */
  static void WriteCanonicalText(const std::filesystem::path& path,
                                 std::string_view content);
};

#endif  // INFRASTRUCTURE_IO_CORE_FILE_WRITER_H_
