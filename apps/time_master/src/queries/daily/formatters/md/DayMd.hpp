// queries/daily/formatters/md/DayMd.hpp
#ifndef DAILY_REPORT_MARKDOWN_FORMATTER_HPP
#define DAILY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/md/DayMdConfig.hpp"
#include <sstream>
#include <memory>

class DayMd : public IReportFormatter<DailyReportData> {
public:
    explicit DayMd(std::shared_ptr<DayMdConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;

    std::shared_ptr<DayMdConfig> config_;
};

#endif // DAILY_REPORT_MARKDOWN_FORMATTER_HPP