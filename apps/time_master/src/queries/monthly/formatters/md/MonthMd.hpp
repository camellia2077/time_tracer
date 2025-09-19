// queries/monthly/formatters/md/MonthMd.hpp
#ifndef MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP
#define MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/interfaces/IReportFormatter.hpp"  
#include "queries/shared/data/MonthlyReportData.hpp"
#include "queries/monthly/formatters/md/MonthMdConfig.hpp"
#include <sstream>
#include <memory>

// Forward declarations
struct MonthlyReportData;
struct sqlite3;

/**
 * @class MonthMd
 * @brief Concrete implementation for formatting monthly report data into a Markdown string.
 */
class MonthMd : public IReportFormatter<MonthlyReportData> {
public:
    explicit MonthMd(std::shared_ptr<MonthMdConfig> config);

    std::string format_report(const MonthlyReportData& data) const override;

private:
    void _display_summary(std::stringstream& ss, const MonthlyReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const MonthlyReportData& data) const;

    std::shared_ptr<MonthMdConfig> config_;
};

#endif // MONTHLY_REPORT_MARKDOWN_FORMATTER_HPP