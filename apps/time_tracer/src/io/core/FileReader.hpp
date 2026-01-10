// io/core/FileReader.hpp
#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <string>
#include <filesystem>

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

#endif // FILE_READER_HPP