// queries/period/formatters/md/PeriodMd.hpp
#ifndef PERIOD_REPORT_MARKDOWN_FORMATTER_HPP
#define PERIOD_REPORT_MARKDOWN_FORMATTER_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/data/PeriodReportData.hpp"
#include "queries/period/formatters/md/PeriodMdConfig.hpp"
#include <sstream>
#include <memory>

// Forward declarations
struct PeriodReportData;
struct sqlite3;

/**
 * @class PeriodMd
 * @brief Concrete implementation for formatting period report data into a Markdown string.
 */
class PeriodMd : public IReportFormatter<PeriodReportData> {
public:
    explicit PeriodMd(std::shared_ptr<PeriodMdConfig> config);

    std::string format_report(const PeriodReportData& data, sqlite3* db) const override;

private:
    void _display_summary(std::stringstream& ss, const PeriodReportData& data) const;
    void _display_project_breakdown(std::stringstream& ss, const PeriodReportData& data, sqlite3* db) const;

    std::shared_ptr<PeriodMdConfig> config_;
};

#endif // PERIOD_REPORT_MARKDOWN_FORMATTER_HPP