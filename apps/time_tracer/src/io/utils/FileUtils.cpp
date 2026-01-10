// io/utils/FileUtils.cpp
#include "io/utils/FileUtils.hpp"
#include <algorithm>
#include <iostream>
#include "common/AnsiColors.hpp" // 假设需要打印警告颜色，如果 common 不可见可移除颜色

namespace fs = std::filesystem;

namespace FileUtils {

std::vector<fs::path> find_files_by_extension_recursively(
    const fs::path& root_path,
    const std::string& extension)
{
    std::vector<fs::path> files_found;
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        return files_found;
    }
    try {
        for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files_found.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error accessing directory " << root_path << ": " << e.what() << std::endl;
    }
    std::sort(files_found.begin(), files_found.end());
    return files_found;
}

// [新增] 实现
std::vector<std::string> resolve_files(
    const std::vector<std::string>& input_paths,
    const std::string& extension) 
{
    std::vector<std::string> resolved_files;
    
    for (const std::string& path_str : input_paths) {
        fs::path p(path_str);
        if (!fs::exists(p)) {
            // 使用 std::cerr 打印警告，如果不想依赖 AnsiColors 可以去掉颜色代码
            std::cerr << YELLOW_COLOR << "Warning: Path does not exist: " << path_str << RESET_COLOR << std::endl;
            continue;
        }

        if (fs::is_regular_file(p)) {
            if (p.extension() == extension) {
                resolved_files.push_back(p.string());
            }
        } else if (fs::is_directory(p)) {
            // 复用现有的递归查找函数
            std::vector<fs::path> found = find_files_by_extension_recursively(p, extension);
            for (const auto& fp : found) {
                resolved_files.push_back(fp.string());
            }
        }
    }
    
    // 去重并排序
    std::sort(resolved_files.begin(), resolved_files.end());
    auto last = std::unique(resolved_files.begin(), resolved_files.end());
    resolved_files.erase(last, resolved_files.end());

    return resolved_files;
}

} // namespace FileUtils