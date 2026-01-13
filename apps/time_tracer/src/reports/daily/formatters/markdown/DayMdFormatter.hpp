// reports/daily/formatters/markdown/DayMdFormatter.hpp
#ifndef DAY_MD_FORMATTER_HPP
#define DAY_MD_FORMATTER_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/model/DailyReportData.hpp"
#include "DayMdConfig.hpp" // 包含正确的头文件
#include <sstream>
#include <memory>

class DayMdFormatter : public IReportFormatter<DailyReportData> {
public:
    // [修正] 使用 DayMdConfig
    explicit DayMdFormatter(std::shared_ptr<DayMdConfig> config);
    std::string format_report(const DailyReportData& data) const override;

private:
    void _display_header(std::stringstream& ss, const DailyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const DailyReportData& data) const;
    void _display_detailed_activities(std::stringstream& ss, const DailyReportData& data) const;

    // [修正] 使用 DayMdConfig
    std::shared_ptr<DayMdConfig> config_;
};

#endif // DAY_MD_FORMATTER_HPP