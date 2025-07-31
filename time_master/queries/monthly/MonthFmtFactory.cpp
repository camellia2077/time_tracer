// queries/report_generators/monthly/formatter/MonthFmtFactory.cpp
#include "MonthFmtFactory.h"
#include "queries/monthly/formatters/MonthMd.h"
#include "queries/monthly/formatters/MonthTex.h" //引入 MonthTex 头文件

#include <stdexcept>

// create_formatter 方法的实现
std::unique_ptr<IMonthFmt> MonthFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<MonthMd>();
        
        // [修改] 新增对 LaTeX 格式的处理
        case ReportFormat::LaTeX:
            return std::make_unique<MonthTex>();

        default:
            // 如果请求了工厂不知道如何创建的格式，则抛出异常
            throw std::invalid_argument("Unsupported report format requested for monthly report.");
    }
}