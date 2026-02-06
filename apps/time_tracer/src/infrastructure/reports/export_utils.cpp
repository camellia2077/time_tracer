// infrastructure/reports/export_utils.cpp
#include "infrastructure/reports/export_utils.hpp"

#include <filesystem>
#include <iostream>

#include "common/ansi_colors.hpp"

namespace fs = std::filesystem;

namespace ExportUtils {

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
      std::cerr << time_tracer::common::colors::kRed << "错误: 不支持的导出格式。" << time_tracer::common::colors::kReset
                << std::endl;
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
      std::cout << time_tracer::common::colors::kGreen << "成功: 共创建 " << files_created << " 个"
                << report_type_name_singular
                << "文件，已保存至: " << final_path.string() << time_tracer::common::colors::kReset
                << std::endl;
    } else {
      std::cout << time_tracer::common::colors::kYellow << "信息: 没有可导出的"
                << report_type_name_singular << "内容。" << time_tracer::common::colors::kReset
                << std::endl;
    }

  } catch (const std::exception& e) {
    // 统一捕获所有异常 (包括 FileSystemHelper 抛出的 runtime_error 和可能的
    // filesystem_error)
    std::cerr << time_tracer::common::colors::kRed << "导出过程中发生错误: " << e.what() << time_tracer::common::colors::kReset
              << std::endl;
  }
}

}  // namespace ExportUtils