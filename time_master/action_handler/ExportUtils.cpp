#include "ExportUtils.h"
#include "common_utils.h" // For color constants
#include <iostream>
#include <filesystem>

// 引入 std::filesystem 命名空间
namespace fs = std::filesystem;

namespace ExportUtils {

std::optional<ReportFormatDetails> get_report_format_details(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return {{"Markdown", ".md"}};
        case ReportFormat::LaTeX:
            return {{"LaTeX", ".tex"}};
        default:
            std::cerr << RED_COLOR << "错误: 不支持的导出格式。" << RESET_COLOR << std::endl;
            return std::nullopt;
    }
}

void execute_export_task(
    const std::string& report_type_name_singular,
    const fs::path& export_root_path,
    const std::function<int()>& file_writing_lambda
) {
    try {
        int files_created = file_writing_lambda();

        if (files_created > 0) {
            fs::path final_path = fs::absolute(export_root_path);
            std::cout << GREEN_COLOR << "成功: 共创建 " << files_created << " 个" << report_type_name_singular 
                      << "文件，已保存至: " << final_path.string() << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "信息: 没有可导出的" << report_type_name_singular << "内容。" << RESET_COLOR << std::endl;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << RED_COLOR << "文件系统错误: " << e.what() << RESET_COLOR << std::endl;
    } catch (const std::exception& e) {
        std::cerr << RED_COLOR << "导出过程中发生未知错误: " << e.what() << RESET_COLOR << std::endl;
    }
}

} // namespace ExportUtils