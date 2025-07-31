// queries/report_generators/daily/formatter/DayFmtFactory.cpp
#include "DayFmtFactory.h"
#include "queries/daily/formatters/DayMd.h"
#include "queries/daily/formatters/DayTex.h" //Tex 格式化器头文件

#include <stdexcept>

std::unique_ptr<IDayFmt> DayFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<DayMd>(); //
        
        case ReportFormat::LaTeX: //  case
            return std::make_unique<DayTex>(); // 创建 DayTex 实例

        // case ReportFormat::Json:
        //     // return std::make_unique<DailyReportJsonFormatter>();
        default:
            // 如果请求了不支持的格式，则抛出异常
            throw std::invalid_argument("Unsupported report format requested."); //
    }
}