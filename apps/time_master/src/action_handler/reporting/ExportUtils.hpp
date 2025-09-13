// action_handler/reporting/ExportUtils.hpp
#ifndef EXPORT_UTILS_HPP
#define EXPORT_UTILS_HPP

// action_handler/ExportUtils.hpp


#include <string>
#include <filesystem>
#include <functional>
#include <optional>
#include "queries/shared/ReportFormat.hpp"  

// 定义导出工具的命名空间
namespace ExportUtils {

// 报告格式的详细信息
struct ReportFormatDetails {
    std::string dir_name;
    std::string extension;
};

/**
 * @brief 根据报告格式枚举返回目录名和文件扩展名。
 * @param format 报告格式枚举。
 * @return 包含格式细节的 std::optional 对象，如果格式不支持则为空。
 */
std::optional<ReportFormatDetails> get_report_format_details(ReportFormat format);

/**
 * @brief 执行文件导出任务的通用执行器。
 * @param report_type_name_singular 报告类型的单数名称 (例如, "日报").
 * @param export_root_path 导出的根路径。
 * @param file_writing_lambda 执行文件写入并返回创建文件数的函数。
 */
void execute_export_task(
    const std::string& report_type_name_singular,
    const std::filesystem::path& export_root_path,
    const std::function<int()>& file_writing_lambda
);

} // namespace ExportUtils

#endif // EXPORT_UTILS_HPP