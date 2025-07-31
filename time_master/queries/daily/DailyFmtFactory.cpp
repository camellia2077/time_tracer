// queries/report_generators/daily/formatter/DailyFmtFactory.cpp
#include "DailyFmtFactory.h"
#include "queries/daily/formatters/DailyMarkdown.h"
#include "queries/daily/formatters/DailyTex.h" //Tex 格式化器头文件

#include <stdexcept>

std::unique_ptr<IDailyFmt> DailyFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<DailyMarkdown>(); //
        
        case ReportFormat::LaTeX: //  case
            return std::make_unique<DailyTex>(); // 创建 DailyTex 实例

        // case ReportFormat::Json:
        //     // return std::make_unique<DailyReportJsonFormatter>();
        default:
            // 如果请求了不支持的格式，则抛出异常
            throw std::invalid_argument("Unsupported report format requested."); //
    }
}