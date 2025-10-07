// reports/daily/formatters/md/DayMd.hpp
#ifndef DAY_MD_HPP
#define DAY_MD_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/md/DayMdConfig.hpp"
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

    std::shared_ptr<DayMdConfig> config_;
};

#endif // DAY_MD_HPP