// action_handler/file/steps/FileCollector.cpp
#include "FileCollector.hpp"
#include "file_handler/FileUtils.hpp"
#include "common/AnsiColors.hpp"
#include <iostream>
#include <algorithm>

bool FileCollector::execute(PipelineContext& context, const std::string& extension) {
    if (!fs::exists(context.input_root)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << context.input_root << RESET_COLOR << std::endl;
        return false;
    }

    context.source_files.clear();
    context.generated_files.clear(); // 清理旧数据

    // 递归查找
    context.source_files = FileUtils::find_files_by_extension_recursively(context.input_root, extension);

    // [逻辑保持] 如果输入本身就是一个符合后缀的文件，也加入列表
    if (fs::is_regular_file(context.input_root) && context.input_root.extension() == extension) {
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