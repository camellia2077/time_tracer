// queries/report_generators/monthly/formatter/MonthlyFormatterFactory.cpp
#include "MonthlyFormatterFactory.h"
#include "queries/formatters/markdown/MonthlyMarkdown.h"
#include "queries/formatters/tex/MonthlyTex.h" //引入 MonthlyTex 头文件

#include <stdexcept>

// create_formatter 方法的实现
std::unique_ptr<IMonthlyReportFormatter> MonthlyFormatterFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<MonthlyMarkdown>();
        
        // [修改] 新增对 LaTeX 格式的处理
        case ReportFormat::LaTeX:
            return std::make_unique<MonthlyTex>();

        default:
            // 如果请求了工厂不知道如何创建的格式，则抛出异常
            throw std::invalid_argument("Unsupported report format requested for monthly report.");
    }
}