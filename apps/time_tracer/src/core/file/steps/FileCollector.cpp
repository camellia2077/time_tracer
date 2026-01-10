// core/file/steps/FileCollector.cpp
#include "FileCollector.hpp"
#include "io/utils/FileUtils.hpp"
#include "io/core/FileSystemHelper.hpp" // [New]
#include "common/AnsiColors.hpp"
#include <iostream>
#include <algorithm>

bool FileCollector::execute(PipelineContext& context, const std::string& extension) {
    // [New] 使用 FileSystemHelper
    if (!FileSystemHelper::exists(context.input_root)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << context.input_root << RESET_COLOR << std::endl;
        return false;
    }

    context.source_files.clear();
    context.generated_files.clear(); 

    // FileUtils 已经在 file_handler 中，保持原样调用
    context.source_files = FileUtils::find_files_by_extension_recursively(context.input_root, extension);

    // [New] 使用 FileSystemHelper
    if (FileSystemHelper::is_regular_file(context.input_root) && context.input_root.extension() == extension) {
        if (std::find(context.source_files.begin(), context.source_files.end(), context.input_root) == context.source_files.end()) {
            context.source_files.push_back(context.input_root);
        }
    }

    if (context.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 在指定路径下没有找到 " << extension << " 文件。" << RESET_COLOR << std::endl;
        return false;
    }

    std::cout << "信息: 成功收集到 " << context.source_files.size() << " 个待处理文件 (" << extension << ")." << std::endl;
    return true;
}