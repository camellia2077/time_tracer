#include "MonthlyReportFormatterFactory.h"
#include "MonthlyReportMarkdownFormatter.h"
// #include "MonthlyReportJsonFormatter.h" // 当未来添加JSON格式时，在此处取消注释
#include <stdexcept>

// [新增] create_formatter 方法的实现
std::unique_ptr<IReportFormatter> MonthlyReportFormatterFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<MonthlyReportMarkdownFormatter>();
        // case ReportFormat::Json:
        //     return std::make_unique<MonthlyReportJsonFormatter>();
        default:
            // 如果请求了工厂不知道如何创建的格式，则抛出异常
            throw std::invalid_argument("Unsupported report format requested for monthly report.");
    }
}