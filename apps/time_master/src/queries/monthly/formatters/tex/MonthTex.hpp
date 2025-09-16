#ifndef MONTHLY_REPORT_TEX_FORMATTER_HPP
#define MONTHLY_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/tex/MonthTexConfig.hpp" // [修改] 引入新的配置类
#include <memory>
#include <sstream>

class MonthTex : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthTex(std::shared_ptr<MonthTexConfig> config); // [修改] 构造函数

    std::string format_report(const MonthlyReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data, sqlite3* db) const;
    
    std::string get_tex_preamble() const;
    std::string get_tex_postfix() const;

    std::shared_ptr<MonthTexConfig> config_; // [新增]
};

#endif // MONTHLY_REPORT_TEX_FORMATTER_HPP