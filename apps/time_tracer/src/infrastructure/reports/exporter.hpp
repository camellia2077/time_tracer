// infrastructure/reports/exporter.hpp
#ifndef INFRASTRUCTURE_REPORTS_EXPORTER_H_
#define INFRASTRUCTURE_REPORTS_EXPORTER_H_

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "reports/data/model/query_data_structs.hpp"
#include "reports/shared/types/report_format.hpp"

namespace fs = std::filesystem;
class ReportFileManager;

class Exporter {
 public:
  explicit Exporter(const fs::path& export_root_path);
  ~Exporter();

  void export_single_day_report(const std::string& date,
                                const std::string& content,
                                ReportFormat format) const;
  void export_single_month_report(const std::string& month,
                                  const std::string& content,
                                  ReportFormat format) const;
  void export_single_period_report(int days, const std::string& content,
                                   ReportFormat format) const;
  void export_single_week_report(const std::string& iso_week,
                                 const std::string& content,
                                 ReportFormat format) const;
  void export_single_year_report(const std::string& year,
                                 const std::string& content,
                                 ReportFormat format) const;

  void export_all_daily_reports(const FormattedGroupedReports& reports,
                                ReportFormat format) const;
  void export_all_monthly_reports(const FormattedMonthlyReports& reports,
                                  ReportFormat format) const;
  void export_all_period_reports(const FormattedPeriodReports& reports,
                                 ReportFormat format) const;
  void export_all_weekly_reports(const FormattedWeeklyReports& reports,
                                 ReportFormat format) const;
  void export_all_yearly_reports(const FormattedYearlyReports& reports,
                                 ReportFormat format) const;

 private:
  static void write_report_to_file(const std::string& report_content,
                                   const fs::path& output_path);
  std::unique_ptr<ReportFileManager> file_manager_;
};
#endif  // INFRASTRUCTURE_REPORTS_EXPORTER_H_
