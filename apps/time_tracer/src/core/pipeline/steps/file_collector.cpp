// core/pipeline/steps/file_collector.cpp
#include "file_collector.hpp"
#include "io/utils/file_utils.hpp"
#include "io/core/file_system_helper.hpp"
#include "common/ansi_colors.hpp"
#include <iostream>
#include <algorithm>

namespace core::pipeline {

bool FileCollector::execute(PipelineContext& context, const std::string& extension) {
    if (!FileSystemHelper::exists(context.config.input_root)) {
        std::cerr << RED_COLOR << "错误: 输入的路径不存在: " << context.config.input_root << RESET_COLOR << std::endl;
        return false;
    }

    context.state.source_files.clear();
    context.state.generated_files.clear(); 

    context.state.source_files = FileUtils::find_files_by_extension_recursively(context.config.input_root, extension);

    if (FileSystemHelper::is_regular_file(context.config.input_root) && context.config.input_root.extension() == extension) {
        if (std::find(context.state.source_files.begin(), context.state.source_files.end(), context.config.input_root) == context.state.source_files.end()) {
            context.state.source_files.push_back(context.config.input_root);
        }
    }

    if (context.state.source_files.empty()) {
        std::cerr << YELLOW_COLOR << "警告: 在指定路径下没有找到 " << extension << " 文件。" << RESET_COLOR << std::endl;
        return false;
    }

    std::cout << "信息: 成功收集到 " << context.state.source_files.size() << " 个待处理文件 (" << extension << ")." << std::endl;
    return true;
}

} // namespace core::pipeline