// infrastructure/reports/export_utils.cpp
#define TT_FORCE_LEGACY_HEADER_DECLS 1
#include "infrastructure/reports/export_utils.hpp"

#include <exception>
#include <filesystem>
#include <optional>
#include <string>

#include "domain/ports/diagnostics.hpp"
#include "shared/types/ansi_colors.hpp"

namespace modports = tracer::core::domain::ports;

namespace modcolors = tracer::core::shared::ansi_colors;

namespace fs = std::filesystem;

namespace tracer::core::infrastructure::reports {

auto GetReportFormatDetails(ReportFormat format)
    -> std::optional<ReportFormatDetails> {
  switch (format) {
    case ReportFormat::kMarkdown:
      return ReportFormatDetails{.dir_name = "markdown", .extension = ".md"};
    case ReportFormat::kLaTeX:
      return ReportFormatDetails{.dir_name = "latex", .extension = ".tex"};
    case ReportFormat::kTyp:
      return ReportFormatDetails{.dir_name = "typ", .extension = ".typ"};

    default:
      modports::EmitError(std::string(modcolors::kRed) +
                          "错误: 不支持的导出格式。" +
                          std::string(modcolors::kReset));
      return std::nullopt;
  }
}

void ExecuteExportTask(const std::string& report_type_name_singular,
                       const fs::path& export_root_path,
                       const std::function<int()>& file_writing_lambda) {
  try {
    int files_created = file_writing_lambda();

    if (files_created > 0) {
      // fs::absolute 可能会抛出 filesystem_error，但会被下方的 std::exception
      // 捕获
      fs::path final_path = fs::absolute(export_root_path);
      modports::EmitInfo(std::string(modcolors::kGreen) + "成功: 共创建 " +
                         std::to_string(files_created) + " 个" +
                         report_type_name_singular + "文件，已保存至: " +
                         final_path.string() + std::string(modcolors::kReset));
    } else {
      modports::EmitWarn(std::string(modcolors::kYellow) +
                         "信息: 没有可导出的" + report_type_name_singular +
                         "内容。" + std::string(modcolors::kReset));
    }

  } catch (const std::exception& e) {
    // 统一捕获所有异常 (包括 FileSystemHelper 抛出的 runtime_error 和可能的
    // filesystem_error)
    modports::EmitError(std::string(modcolors::kRed) + "导出过程中发生错误: " +
                        e.what() + std::string(modcolors::kReset));
  }
}

}  // namespace tracer::core::infrastructure::reports
