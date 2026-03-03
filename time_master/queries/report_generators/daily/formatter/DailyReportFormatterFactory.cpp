// queries/report_generators/daily/formatter/DailyReportFormatterFactory.cpp
#include "DailyReportFormatterFactory.h"
#include "queries/report_generators/daily/formatter/day_md/DailyMarkdown.h"
#include "queries/report_generators/daily/formatter/day_tex/DailyTex.h" //Tex 格式化器头文件

#include <stdexcept>

std::unique_ptr<IReportFormatter> DailyReportFormatterFactory::create_formatter(ReportFormat format) {
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