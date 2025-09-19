// queries/daily/formatters/tex/DayTex.hpp
#ifndef DAILY_REPORT_TEX_FORMATTER_HPP
#define DAILY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/DailyReportData.hpp"
#include "queries/daily/formatters/tex/DayTexConfig.hpp"
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
    
    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_tex_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const;

    std::shared_ptr<DayTexConfig> config_;
};

#endif // DAILY_REPORT_TEX_FORMATTER_HPP