#include "PeriodReportFormatterFactory.h"
#include "PeriodReportMarkdownFormatter.h"
// #include "PeriodReportJsonFormatter.h" // 未来若支持JSON，在此处添加
#include <stdexcept>

// create_formatter 方法的实现
std::unique_ptr<IReportFormatter> PeriodReportFormatterFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<PeriodReportMarkdownFormatter>();
        // case ReportFormat::Json:
        //     return std::make_unique<PeriodReportJsonFormatter>();
        default:
            throw std::invalid_argument("Unsupported report format requested for period report.");
    }
}