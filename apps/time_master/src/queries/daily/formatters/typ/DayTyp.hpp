#ifndef DAY_TYP_HPP
#define DAY_TYP_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"
#include <memory>
#include <sstream>

class DayTyp : public IReportFormatter<DailyReportData> {
public:
    explicit DayTyp(std::shared_ptr<DayTypConfig> config);
    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;
    std::string _format_activity_line(const TimeRecord& record) const;

    std::shared_ptr<DayTypConfig> config_;
};

#endif // DAY_TYP_HPP