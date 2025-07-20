#include "DailyReportFormatterFactory.h"
#include "DailyReportMarkdownFormatter.h"
// #include "DailyReportJsonFormatter.h" // 未来可添加
#include <stdexcept>

std::unique_ptr<IReportFormatter> DailyReportFormatterFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<DailyReportMarkdownFormatter>();
        // case ReportFormat::Json:
        //     return std::make_unique<DailyReportJsonFormatter>();
        default:
            // 如果未来加入了更多格式却忘记在工厂中实现，这里会抛出异常
            throw std::invalid_argument("Unsupported report format requested.");
    }
}