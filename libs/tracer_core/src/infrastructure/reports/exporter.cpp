// infrastructure/reports/exporter.cpp
#include "infrastructure/reports/exporter.hpp"

#include <iomanip>
#include <sstream>

#include "domain/ports/diagnostics.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/core/file_writer.hpp"
#include "infrastructure/reports/export_utils.hpp"
#include "infrastructure/reports/report_file_manager.hpp"
#include "shared/types/ansi_colors.hpp"

Exporter::Exporter(const fs::path& export_root_path) {
  file_manager_ = std::make_unique<ReportFileManager>(export_root_path);
}

Exporter::~Exporter() = default;

namespace {
constexpr int kSingleDigitMonthThreshold = 10;
}  // namespace

void Exporter::WriteReportToFile(std::string_view report_content,
                                 const fs::path& output_path) {
  if (report_content.empty() ||
      report_content.find("No time records") != std::string_view::npos) {
    return;
  }

  fs::path output_dir = output_path.parent_path();
  try {
    // [New] 使用 FileSystemHelper 创建目录
    FileSystemHelper::CreateDirectories(output_dir);

    // [New] 使用 FileWriter 写入文件
    FileWriter::WriteContent(output_path, std::string(report_content));

  } catch (const std::exception& e) {
    tracer_core::domain::ports::EmitError(
        std::string(tracer_core::common::colors::kRed) +
        "Error: Failed to write report to " + output_path.string() + ": " +
        e.what() + std::string(tracer_core::common::colors::kReset));
  }
}

// ... (其余代码保持不变) ...

void Exporter::ExportSingleDayReport(const SingleExportTask& task,
                                     ReportFormat format) const {
  fs::path path =
      file_manager_->GetSingleDayReportPath(std::string(task.id), format);
  tracer_core::domain::ports::EmitInfo(
      std::string(tracer_core::common::colors::kGreen) +
      "Success: Report exported to " + fs::absolute(path).string() +
      std::string(tracer_core::common::colors::kReset));
  WriteReportToFile(task.kContent, path);
}

void Exporter::ExportSingleMonthReport(const SingleExportTask& task,
                                       ReportFormat format) const {
  fs::path path =
      file_manager_->GetSingleMonthReportPath(std::string(task.id), format);
  tracer_core::domain::ports::EmitInfo(
      std::string(tracer_core::common::colors::kGreen) +
      "Success: Report exported to " + fs::absolute(path).string() +
      std::string(tracer_core::common::colors::kReset));
  WriteReportToFile(task.kContent, path);
}

void Exporter::ExportSinglePeriodReport(int days, std::string_view content,
                                        ReportFormat format) const {
  fs::path path = file_manager_->GetSinglePeriodReportPath(days, format);
  tracer_core::domain::ports::EmitInfo(
      std::string(tracer_core::common::colors::kGreen) +
      "Success: Report exported to " + fs::absolute(path).string() +
      std::string(tracer_core::common::colors::kReset));
  WriteReportToFile(content, path);
}

void Exporter::ExportSingleWeekReport(const SingleExportTask& task,
                                      ReportFormat format) const {
  fs::path path =
      file_manager_->GetSingleWeekReportPath(std::string(task.id), format);
  tracer_core::domain::ports::EmitInfo(
      std::string(tracer_core::common::colors::kGreen) +
      "Success: Report exported to " + fs::absolute(path).string() +
      std::string(tracer_core::common::colors::kReset));
  WriteReportToFile(task.kContent, path);
}

void Exporter::ExportSingleYearReport(const SingleExportTask& task,
                                      ReportFormat format) const {
  fs::path path =
      file_manager_->GetSingleYearReportPath(std::string(task.id), format);
  tracer_core::domain::ports::EmitInfo(
      std::string(tracer_core::common::colors::kGreen) +
      "Success: Report exported to " + fs::absolute(path).string() +
      std::string(tracer_core::common::colors::kReset));
  WriteReportToFile(task.kContent, path);
}

void Exporter::ExportAllDailyReports(const FormattedGroupedReports& reports,
                                     ReportFormat format) const {
  fs::path base_dir = file_manager_->GetAllDailyReportsBaseDir(format);

  ExportUtils::ExecuteExportTask("日报", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& year_pair : reports) {
      for (const auto& month_pair : year_pair.second) {
        for (const auto& day_report : month_pair.second) {
          const std::string& date = day_report.report_id;
          const std::string& content = day_report.kContent;
          fs::path report_path =
              file_manager_->GetSingleDayReportPath(date, format);
          WriteReportToFile(content, report_path);
          if (!content.empty()) {
            files_created++;
          }
        }
      }
    }
    return files_created;
  });
}

void Exporter::ExportAllMonthlyReports(const FormattedMonthlyReports& reports,
                                       ReportFormat format) const {
  fs::path base_dir = file_manager_->GetAllMonthlyReportsBaseDir(format);

  ExportUtils::ExecuteExportTask("月报", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& year_pair : reports) {
      int year = year_pair.first;
      for (const auto& month_pair : year_pair.second) {
        int month = month_pair.first;
        const std::string& content = month_pair.second;
        std::string month_str =
            std::to_string(year) +
            (month < kSingleDigitMonthThreshold ? "0" : "") +
            std::to_string(month);

        fs::path report_path =
            file_manager_->GetSingleMonthReportPath(month_str, format);
        WriteReportToFile(content, report_path);
        if (!content.empty()) {
          files_created++;
        }
      }
    }
    return files_created;
  });
}

void Exporter::ExportAllPeriodReports(const FormattedPeriodReports& reports,
                                      ReportFormat format) const {
  fs::path base_dir = file_manager_->GetAllPeriodReportsBaseDir(format);

  ExportUtils::ExecuteExportTask("周期报告", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& report_pair : reports) {
      int days = report_pair.first;
      const std::string& content = report_pair.second;
      fs::path report_path =
          file_manager_->GetSinglePeriodReportPath(days, format);
      WriteReportToFile(content, report_path);
      if (!content.empty()) {
        files_created++;
      }
    }
    return files_created;
  });
}

void Exporter::ExportAllWeeklyReports(const FormattedWeeklyReports& reports,
                                      ReportFormat format) const {
  fs::path base_dir = file_manager_->GetAllWeeklyReportsBaseDir(format);

  ExportUtils::ExecuteExportTask("鍛ㄦ姤", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& year_pair : reports) {
      int year = year_pair.first;
      for (const auto& week_pair : year_pair.second) {
        int week = week_pair.first;
        const std::string& content = week_pair.second;
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << year << "-W" << std::setw(2)
            << std::setfill('0') << week;
        std::string week_label = oss.str();

        fs::path report_path =
            file_manager_->GetSingleWeekReportPath(week_label, format);
        WriteReportToFile(content, report_path);
        if (!content.empty()) {
          files_created++;
        }
      }
    }
    return files_created;
  });
}

void Exporter::ExportAllYearlyReports(const FormattedYearlyReports& reports,
                                      ReportFormat format) const {
  fs::path base_dir = file_manager_->GetAllYearlyReportsBaseDir(format);

  ExportUtils::ExecuteExportTask("骞存姤", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& [year, content] : reports) {
      std::ostringstream oss;
      oss << std::setw(4) << std::setfill('0') << year;
      std::string year_label = oss.str();

      fs::path report_path =
          file_manager_->GetSingleYearReportPath(year_label, format);
      WriteReportToFile(content, report_path);
      if (!content.empty()) {
        files_created++;
      }
    }
    return files_created;
  });
}
