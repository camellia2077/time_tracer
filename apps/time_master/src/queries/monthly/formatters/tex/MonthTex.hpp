// queries/monthly/formatters/tex/MonthTex.hpp
#ifndef MONTHLY_REPORT_TEX_FORMATTER_HPP
#define MONTHLY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp"
#include <memory>
#include <sstream>

class MonthTex : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTex(std::shared_ptr<MonthTexConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;
    
    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_tex_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const;

    std::shared_ptr<MonthTexConfig> config_;
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_HPP