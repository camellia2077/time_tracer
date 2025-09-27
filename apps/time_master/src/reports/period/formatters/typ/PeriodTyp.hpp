// reports/period/formatters/typ/PeriodTyp.hpp
#ifndef PERIOD_TYP_HPP
#define PERIOD_TYP_HPP

#include "reports/shared/interfaces/IReportFormatter.hpp"
#include "reports/shared/data/PeriodReportData.hpp"
#include "reports/period/formatters/typ/PeriodTypConfig.hpp"
#include <sstream>
#include <memory>

class PeriodTyp : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTyp(std::shared_ptr<PeriodTypConfig> config); 
    std::string format_report(const PeriodReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data) const;

    // [新增] 内部方法，用于格式化项目树
    std::string _format_project_tree(const ProjectTree& tree, long long total_duration, int avg_days) const;
    void _generate_sorted_typ_output(std::stringstream& ss, const ProjectNode& node, int indent, int avg_days) const;

    std::shared_ptr<PeriodTypConfig> config_;
};

#endif // PERIOD_TYP_HPP