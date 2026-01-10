// io/core/FileWriter.hpp
#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <string>
#include <filesystem>

/**
 * @brief 负责文件写入操作的静态工具类。
 */
class FileWriter {
public:
    /**
     * @brief 将字符串内容写入指定文件。
     * @details 如果文件已存在则覆盖，如果不存在则创建（前提是目录存在）。
     * @param path 目标文件路径。
     * @param content 要写入的内容。
     * @throws std::runtime_error 如果文件无法打开或写入失败。
     */
    static void write_content(const std::filesystem::path& path, const std::string& content);
};

#endif // FILE_WRITER_HPP