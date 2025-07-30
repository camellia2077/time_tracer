#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <vector>
#include <filesystem>
#include <string>

// file_handler/FileUtils.h

// 使用 namespace 来组织通用的、无状态的辅助函数
namespace FileUtils {

    /**
     * @brief 递归地查找指定目录及其所有子目录下的特定扩展名文件。
     * @param root_path 要开始搜索的根目录。
     * @param extension 要查找的文件扩展名 (例如, ".txt")。
     * @return 一个包含所有找到的文件路径的向量(std::vector)，并已按字母顺序排序。
     */
    std::vector<std::filesystem::path> find_files_by_extension_recursively(
        const std::filesystem::path& root_path, 
        const std::string& extension
    );

} // namespace FileUtils

#endif // FILE_UTILS_H