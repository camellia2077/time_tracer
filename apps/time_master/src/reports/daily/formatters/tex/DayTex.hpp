// reports/daily/formatters/tex/DayTex.hpp
#ifndef DAY_TEX_HPP
#define DAY_TEX_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/DailyReportData.hpp"
#include "reports/daily/formatters/tex/DayTexConfig.hpp"
#include <memory>
#include <sstream>

class DayTex : public IReportFormatter<DailyReportData> {
public:
    explicit DayTex(std::shared_ptr<DayTexConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
    
    std::shared_ptr<DayTexConfig> config_;
};

#endif // DAY_TEX_HPP