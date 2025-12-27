// action_handler/reporting/Exporter.hpp
#ifndef EXPORTER_HPP
#define EXPORTER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include "reports/shared/types/ReportFormat.hpp"
#include "reports/shared/data/query_data_structs.hpp"

namespace fs = std::filesystem;
class ReportFileManager;

class Exporter {
public:
    explicit Exporter(const fs::path& export_root_path);
    ~Exporter();

    void export_single_day_report(const std::string& date, const std::string& content, ReportFormat format) const;
    void export_single_month_report(const std::string& month, const std::string& content, ReportFormat format) const;
    void export_single_period_report(int days, const std::string& content, ReportFormat format) const;

    void export_all_daily_reports(const FormattedGroupedReports& reports, ReportFormat format) const;
    void export_all_monthly_reports(const FormattedMonthlyReports& reports, ReportFormat format) const;
    void export_all_period_reports(const FormattedPeriodReports& reports, ReportFormat format) const;

private:
    void write_report_to_file(const std::string& report_content, const fs::path& output_path) const;
    std::unique_ptr<ReportFileManager> file_manager_;
};
#endif // EXPORTER_HPP