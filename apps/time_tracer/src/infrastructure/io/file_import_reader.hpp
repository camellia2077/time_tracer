// infrastructure/io/file_import_reader.hpp
#ifndef INFRASTRUCTURE_IO_FILE_IMPORT_READER_H_
#define INFRASTRUCTURE_IO_FILE_IMPORT_READER_H_

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "infrastructure/io/core/file_reader.hpp"
#include "infrastructure/io/utils/file_utils.hpp"
#include "shared/types/ansi_colors.hpp"

namespace infrastructure::io {

class FileImportReader {
 public:
  /**
   * @brief 扫描指定路径下的 .json 文件并读取内容
   * @return pair 列表: <文件名, 文件内容>
   */
  static auto ReadJsonFiles(const std::string& path_str)
      -> std::vector<std::pair<std::string, std::string>> {
    std::cout << "正在扫描待导入文件..." << std::endl;
    std::vector<std::string> input_paths = {path_str};
    // 职责分离：调用现有的工具类
    std::vector<std::string> json_files =
        FileUtils::ResolveFiles(input_paths, ".json");

    if (json_files.empty()) {
      std::cerr << time_tracer::common::colors::kYellow << "警告: 在路径 "
                << path_str << " 下未找到 .json 文件。"
                << time_tracer::common::colors::kReset << std::endl;
      return {};
    }

    std::cout << "正在读取 " << json_files.size() << " 个文件的内容..."
              << std::endl;
    std::vector<std::pair<std::string, std::string>> payload;
    payload.reserve(json_files.size());

    int read_failure_count = 0;
    for (const auto& file_path : json_files) {
      try {
        std::string content = FileReader::ReadContent(file_path);
        payload.emplace_back(file_path, std::move(content));
      } catch (const std::exception& e) {
        std::cerr << time_tracer::common::colors::kRed
                  << "读取失败: " << file_path << " - " << e.what()
                  << time_tracer::common::colors::kReset << std::endl;
        read_failure_count++;
      }
    }

    if (read_failure_count > 0) {
      std::cout << time_tracer::common::colors::kYellow << "警告: 有 "
                << read_failure_count << " 个文件读取失败，将跳过这些文件。"
                << time_tracer::common::colors::kReset << std::endl;
    }

    return payload;
  }
};

}  // namespace infrastructure::io

#endif  // INFRASTRUCTURE_IO_FILE_IMPORT_READER_H_
