// queries/monthly/formatters/typ/MonthTyp.hpp
#ifndef MONTHLY_REPORT_TYP_FORMATTER_HPP
#define MONTHLY_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/typ/MonthTypConfig.hpp"
#include <sstream>
#include <memory>

class MonthTyp : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTyp(std::shared_ptr<MonthTypConfig> config);
    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const;

    std::shared_ptr<MonthTypConfig> config_;
};

#endif // MONTHLY_REPORT_TYP_FORMATTER_HPP