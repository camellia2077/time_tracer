// queries/period/formatters/tex/PeriodTex.hpp
#ifndef PERIOD_REPORT_TEX_FORMATTER_HPP
#define PERIOD_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"  
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"
#include <memory>
#include <sstream>

class PeriodTex : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTex(std::shared_ptr<PeriodTexConfig> config);
    std::string format_report(const PeriodReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const;

    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_tex_output(std::stringstream& ss, const ProjectNode& node, int avg_days) const;

    std::shared_ptr<PeriodTexConfig> config_;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_HPP