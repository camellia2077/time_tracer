
#include "FileUtils.h"
#include <algorithm> // for std::sort

// file_handler/FileUtils.cpp

namespace fs = std::filesystem;

namespace FileUtils {

std::vector<fs::path> find_files_by_extension_recursively(
    const fs::path& root_path, 
    const std::string& extension) 
{
    std::vector<fs::path> files_found;
    
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        // 如果路径不存在或不是一个目录，则直接返回空向量
        return files_found; 
    }

    // 使用 recursive_directory_iterator 进行递归遍历
    for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
        if (entry.is_regular_file() && entry.path().extension() == extension) {
            files_found.push_back(entry.path());
        }
    }

    // 对找到的文件进行排序，确保处理顺序一致
    std::sort(files_found.begin(), files_found.end());

    return files_found;
}

} // namespace FileUtils