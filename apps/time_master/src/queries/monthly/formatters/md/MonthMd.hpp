// queries/monthly/formatters/md/MonthMd.hpp
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"  
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/md/MonthMdConfig.hpp"
#include <sstream>
#include <memory>

class MonthMd : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthMd(std::shared_ptr<MonthMdConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_md_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const;

    std::shared_ptr<MonthMdConfig> config_;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP