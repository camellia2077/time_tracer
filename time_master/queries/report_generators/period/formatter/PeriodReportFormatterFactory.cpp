#include "PeriodReportFormatterFactory.h"
#include "queries/report_generators/period/formatter/period_md/PeriodReportMarkdownFormatter.h"
#include "queries/report_generators/period/formatter/period_tex/PeriodTex.h"


#include <stdexcept>

// create_formatter 方法的实现
std::unique_ptr<IReportFormatter> PeriodReportFormatterFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<PeriodReportMarkdownFormatter>();
        case ReportFormat::LaTeX:
            return std::make_unique<PeriodTex>();
        // case ReportFormat::Json:
        //     return std::make_unique<PeriodReportJsonFormatter>();
        default:
            throw std::invalid_argument("Unsupported report format requested for period report.");
    }
}