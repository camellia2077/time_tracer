// queries/period/formatters/md/PeriodMd.hpp
#ifndef PERIOD_REPORT_MARKDOWN_FORMATTER_HPP
#define PERIOD_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include <sstream>
#include <memory>

class PeriodMd : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodMd(std::shared_ptr<PeriodMdConfig> config);
    std::string format_report(const PeriodReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const;

    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_md_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const;

    std::shared_ptr<PeriodMdConfig> config_;
};

#endif // PERIOD_REPORT_MARKDOWN_FORMATTER_HPP