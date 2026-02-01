// infrastructure/reports/exporter.cpp
#include "exporter.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

#include "application/reporting/generator/report_file_manager.hpp"
#include "common/ansi_colors.hpp"
#include "infrastructure/io/core/file_system_helper.hpp"
#include "infrastructure/io/core/file_writer.hpp"
#include "infrastructure/reports/export_utils.hpp"

Exporter::Exporter(const fs::path& export_root_path) {
  file_manager_ = std::make_unique<ReportFileManager>(export_root_path);
}

Exporter::~Exporter() = default;

void Exporter::write_report_to_file(const std::string& report_content,
                                    const fs::path& output_path) {
  if (report_content.empty() ||
      report_content.find("No time records") != std::string::npos) {
    return;
  }

  fs::path output_dir = output_path.parent_path();
  try {
    // [New] 使用 FileSystemHelper 创建目录
    FileSystemHelper::create_directories(output_dir);

    // [New] 使用 FileWriter 写入文件
    FileWriter::write_content(output_path, report_content);

  } catch (const std::exception& e) {
    std::cerr << RED_COLOR << "Error: Failed to write report to " << output_path
              << ": " << e.what() << RESET_COLOR << std::endl;
  }
}

// ... (其余代码保持不变) ...

void Exporter::export_single_day_report(const std::string& date,
                                        const std::string& content,
                                        ReportFormat format) const {
  fs::path path = file_manager_->get_single_day_report_path(date, format);
  std::cout << GREEN_COLOR << "Success: Report exported to "
            << fs::absolute(path) << RESET_COLOR << std::endl;
  write_report_to_file(content, path);
}

void Exporter::export_single_month_report(const std::string& month,
                                          const std::string& content,
                                          ReportFormat format) const {
  fs::path path = file_manager_->get_single_month_report_path(month, format);
  std::cout << GREEN_COLOR << "Success: Report exported to "
            << fs::absolute(path) << RESET_COLOR << std::endl;
  write_report_to_file(content, path);
}

void Exporter::export_single_period_report(int days, const std::string& content,
                                           ReportFormat format) const {
  fs::path path = file_manager_->get_single_period_report_path(days, format);
  std::cout << GREEN_COLOR << "Success: Report exported to "
            << fs::absolute(path) << RESET_COLOR << std::endl;
  write_report_to_file(content, path);
}

void Exporter::export_single_week_report(const std::string& iso_week,
                                         const std::string& content,
                                         ReportFormat format) const {
  fs::path path = file_manager_->get_single_week_report_path(iso_week, format);
  std::cout << GREEN_COLOR << "Success: Report exported to "
            << fs::absolute(path) << RESET_COLOR << std::endl;
  write_report_to_file(content, path);
}

void Exporter::export_single_year_report(const std::string& year,
                                         const std::string& content,
                                         ReportFormat format) const {
  fs::path path = file_manager_->get_single_year_report_path(year, format);
  std::cout << GREEN_COLOR << "Success: Report exported to "
            << fs::absolute(path) << RESET_COLOR << std::endl;
  write_report_to_file(content, path);
}

void Exporter::export_all_daily_reports(const FormattedGroupedReports& reports,
                                        ReportFormat format) const {
  fs::path base_dir = file_manager_->get_all_daily_reports_base_dir(format);

  ExportUtils::execute_export_task("日报", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& year_pair : reports) {
      for (const auto& month_pair : year_pair.second) {
        for (const auto& day_report : month_pair.second) {
          const std::string& date = day_report.first;
          const std::string& content = day_report.second;
          fs::path report_path =
              file_manager_->get_single_day_report_path(date, format);
          write_report_to_file(content, report_path);
          if (!content.empty()) {
            files_created++;
          }
        }
      }
    }
    return files_created;
  });
}

void Exporter::export_all_monthly_reports(
    const FormattedMonthlyReports& reports, ReportFormat format) const {
  fs::path base_dir = file_manager_->get_all_monthly_reports_base_dir(format);

  ExportUtils::execute_export_task("月报", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& year_pair : reports) {
      int year = year_pair.first;
      for (const auto& month_pair : year_pair.second) {
        int month = month_pair.first;
        const std::string& content = month_pair.second;
        std::string month_str = std::to_string(year) + (month < 10 ? "0" : "") +
                                std::to_string(month);

        fs::path report_path =
            file_manager_->get_single_month_report_path(month_str, format);
        write_report_to_file(content, report_path);
        if (!content.empty()) {
          files_created++;
        }
      }
    }
    return files_created;
  });
}

void Exporter::export_all_period_reports(const FormattedPeriodReports& reports,
                                         ReportFormat format) const {
  fs::path base_dir = file_manager_->get_all_period_reports_base_dir(format);

  ExportUtils::execute_export_task("周期报告", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& report_pair : reports) {
      int days = report_pair.first;
      const std::string& content = report_pair.second;
      fs::path report_path =
          file_manager_->get_single_period_report_path(days, format);
      write_report_to_file(content, report_path);
      if (!content.empty()) {
        files_created++;
      }
    }
    return files_created;
  });
}

void Exporter::export_all_weekly_reports(const FormattedWeeklyReports& reports,
                                         ReportFormat format) const {
  fs::path base_dir = file_manager_->get_all_weekly_reports_base_dir(format);

  ExportUtils::execute_export_task("鍛ㄦ姤", base_dir, [&]() -> int {
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
            file_manager_->get_single_week_report_path(week_label, format);
        write_report_to_file(content, report_path);
        if (!content.empty()) {
          files_created++;
        }
      }
    }
    return files_created;
  });
}

void Exporter::export_all_yearly_reports(const FormattedYearlyReports& reports,
                                         ReportFormat format) const {
  fs::path base_dir = file_manager_->get_all_yearly_reports_base_dir(format);

  ExportUtils::execute_export_task("骞存姤", base_dir, [&]() -> int {
    int files_created = 0;
    for (const auto& [year, content] : reports) {
      std::ostringstream oss;
      oss << std::setw(4) << std::setfill('0') << year;
      std::string year_label = oss.str();

      fs::path report_path =
          file_manager_->get_single_year_report_path(year_label, format);
      write_report_to_file(content, report_path);
      if (!content.empty()) {
        files_created++;
      }
    }
    return files_created;
  });
}
