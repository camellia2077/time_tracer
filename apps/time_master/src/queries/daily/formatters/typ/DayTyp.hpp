// queries/daily/formatters/typ/DayTyp.hpp
#ifndef DAILY_REPORT_TYP_FORMATTER_H
#define DAILY_REPORT_TYP_FORMATTER_H

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/typ/DayTypConfig.hpp"

#include <sstream>
#include <memory>

// Forward declaration
struct DailyReportData;

class DayTyp : public IReportFormatter<DailyReportData> {
public:
    explicit DayTyp(std::shared_ptr<DayTypConfig> config);

    std::string format_report(const DailyReportData& data, sqlite3* db) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data, sqlite3* db) const;
    void _display_statistics(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;

    /**
     * @brief 格式化单行活动记录，为特定关键词应用颜色。
     * @param record 单个时间记录。
     * @return 格式化后的 Typst 字符串。
     */
    std::string _format_activity_line(const TimeRecord& record) const;

    std::shared_ptr<DayTypConfig> config_;
};

#endif // DAILY_REPORT_TYP_FORMATTER_H