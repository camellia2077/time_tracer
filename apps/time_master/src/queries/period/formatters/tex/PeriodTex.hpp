#ifndef PERIOD_REPORT_TEX_FORMATTER_HPP
#define PERIOD_REPORT_TEX_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"  
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/tex/PeriodTexConfig.hpp"
#include <memory>
#include <sstream>

class PeriodTex : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodTex(std::shared_ptr<PeriodTexConfig> config);

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;

    std::string get_tex_preamble() const;
    std::string get_tex_postfix() const;
    
    std::shared_ptr<PeriodTexConfig> config_;
};

#endif // PERIOD_REPORT_TEX_FORMATTER_HPP