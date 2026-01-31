// io/utils/file_utils.cpp
#include "io/utils/file_utils.hpp"

#include <algorithm>
#include <iostream>

#include "common/ansi_colors.hpp"

namespace fs = std::filesystem;

namespace FileUtils {

auto find_files_by_extension_recursively(const fs::path& root_path,
                                         const std::string& extension)
    -> std::vector<fs::path> {
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
    std::cerr << "Filesystem error accessing directory " << root_path << ": "
              << e.what() << std::endl;
  }
  std::ranges::sort(files_found);
  return files_found;
}

// [新增] 实现
auto resolve_files(const std::vector<std::string>& input_paths,
                   const std::string& extension) -> std::vector<std::string> {
  std::vector<std::string> resolved_files;

  for (const std::string& path_str : input_paths) {
    fs::path p(path_str);
    if (!fs::exists(p)) {
      // 使用 std::cerr 打印警告，如果不想依赖 AnsiColors 可以去掉颜色代码
      std::cerr << YELLOW_COLOR << "Warning: Path does not exist: " << path_str
                << RESET_COLOR << std::endl;
      continue;
    }

    if (fs::is_regular_file(p)) {
      if (p.extension() == extension) {
        resolved_files.push_back(p.string());
      }
    } else if (fs::is_directory(p)) {
      // 复用现有的递归查找函数
      std::vector<fs::path> found =
          find_files_by_extension_recursively(p, extension);
      for (const auto& fp : found) {
        resolved_files.push_back(fp.string());
      }
    }
  }

  // 去重并排序
  std::ranges::sort(resolved_files);
  auto sub = std::ranges::unique(resolved_files);
  resolved_files.erase(sub.begin(), sub.end());

  return resolved_files;
}

}  // namespace FileUtils