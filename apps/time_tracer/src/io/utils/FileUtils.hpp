// io/utils/FileUtils.hpp
#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <vector>
#include <filesystem>
#include <string>

namespace FileUtils {

    /**
     * @brief 递归地查找指定目录及其所有子目录下的特定扩展名文件。
     * @param root_path 要开始搜索的根目录。
     * @param extension 要查找的文件扩展名 (例如, ".txt")。
     * @return 一个包含所有找到的文件路径的向量。
     */
    std::vector<std::filesystem::path> find_files_by_extension_recursively(
        const std::filesystem::path& root_path,
        const std::string& extension
    );

    /**
     * @brief [新增] 解析输入路径列表，将其中的目录展开为文件，并保留直接指定的文件。
     * @details 这是一个高层辅助函数，逻辑迁移自原 db_inserter/PathScanner。
     * @param input_paths 输入的路径列表（可以是文件或目录）。
     * @param extension 要筛选的后缀名（例如 ".json"）。
     * @return 所有符合条件的文件路径列表（已排序）。
     */
    std::vector<std::string> resolve_files(
        const std::vector<std::string>& input_paths,
        const std::string& extension
    );

} // namespace FileUtils

#endif // FILE_UTILS_HPP