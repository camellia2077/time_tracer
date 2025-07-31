#include "PeriodFmtFactory.h"
#include "queries/period/formatters/PeriodMarkdown.h" // Corrected header
#include "queries/period/formatters/PeriodTex.h"

#include <stdexcept>

// create_formatter 方法的实现
std::unique_ptr<IPeriodFmt> PeriodFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<PeriodMarkdown>();
        case ReportFormat::LaTeX:
            return std::make_unique<PeriodTex>();
        // case ReportFormat::Json:
        //     return std::make_unique<PeriodReportJsonFormatter>();
        default:
            throw std::invalid_argument("Unsupported report format requested for period report.");
    }
}