// queries/monthly/formatters/typ/MonthTyp.hpp
#ifndef MONTHLY_REPORT_TYP_FORMATTER_HPP
#define MONTHLY_REPORT_TYP_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/typ/MonthTypConfig.hpp" // [新增]
#include <sstream>
#include <memory> // [新增]

class MonthTyp : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTyp(std::shared_ptr<MonthTypConfig> config); // [修改] 构造函数接收配置

    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    std::shared_ptr<MonthTypConfig> config_; // [新增] 保存配置的智能指针
};

#endif // MONTHLY_REPORT_TYP_FORMATTER_HPP