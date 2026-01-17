// core/importer/FileImportReader.hpp
#ifndef CORE_IMPORTER_FILE_IMPORT_READER_HPP_
#define CORE_IMPORTER_FILE_IMPORT_READER_HPP_

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include "io/utils/FileUtils.hpp"
#include "io/core/FileReader.hpp"
#include "common/AnsiColors.hpp"

class FileImportReader {
public:
    /**
     * @brief 扫描指定路径下的 .json 文件并读取内容
     * @return pair 列表: <文件名, 文件内容>
     */
    static std::vector<std::pair<std::string, std::string>> read_json_files(const std::string& path_str) {
        std::cout << "正在扫描待导入文件..." << std::endl;
        std::vector<std::string> input_paths = {path_str};
        // 职责分离：调用现有的工具类
        std::vector<std::string> json_files = FileUtils::resolve_files(input_paths, ".json");

        if (json_files.empty()) {
            std::cerr << YELLOW_COLOR << "警告: 在路径 " << path_str << " 下未找到 .json 文件。" << RESET_COLOR << std::endl;
            return {};
        }

        std::cout << "正在读取 " << json_files.size() << " 个文件的内容..." << std::endl;
        std::vector<std::pair<std::string, std::string>> payload;
        payload.reserve(json_files.size());

        int read_failure_count = 0;
        for (const auto& file_path : json_files) {
            try {
                std::string content = FileReader::read_content(file_path);
                payload.push_back({file_path, std::move(content)});
            } catch (const std::exception& e) {
                std::cerr << RED_COLOR << "读取失败: " << file_path << " - " << e.what() << RESET_COLOR << std::endl;
                read_failure_count++;
            }
        }

        if (read_failure_count > 0) {
            std::cout << YELLOW_COLOR << "警告: 有 " << read_failure_count << " 个文件读取失败，将跳过这些文件。" << RESET_COLOR << std::endl;
        }

        return payload;
    }
};

#endif // CORE_IMPORTER_FILE_IMPORT_READER_HPP_