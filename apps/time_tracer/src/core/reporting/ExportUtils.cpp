// core/reporting/ExportUtils.cpp
#include "ExportUtils.hpp"
#include "common/AnsiColors.hpp" 
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace ExportUtils {

std::optional<ReportFormatDetails> get_report_format_details(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return ReportFormatDetails{"Markdown_logs", ".md"};
        case ReportFormat::LaTeX:
            return ReportFormatDetails{"LaTeX_logs", ".tex"};
        case ReportFormat::Typ:
            return ReportFormatDetails{"Typst_logs", ".typ"};
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
            // fs::absolute 可能会抛出 filesystem_error，但会被下方的 std::exception 捕获
            fs::path final_path = fs::absolute(export_root_path);
            std::cout << GREEN_COLOR << "成功: 共创建 " << files_created << " 个" << report_type_name_singular 
                      << "文件，已保存至: " << final_path.string() << RESET_COLOR << std::endl;
        } else {
             std::cout << YELLOW_COLOR << "信息: 没有可导出的" << report_type_name_singular << "内容。" << RESET_COLOR << std::endl;
        }

    } catch (const std::exception& e) {
        // 统一捕获所有异常 (包括 FileSystemHelper 抛出的 runtime_error 和可能的 filesystem_error)
        std::cerr << RED_COLOR << "导出过程中发生错误: " << e.what() << RESET_COLOR << std::endl;
    }
}

} // namespace ExportUtils