#include "common/pch.h"
// queries/report_generators/daily/formatter/DayFmtFactory.cpp
#include "DayFmtFactory.h"
#include "queries/daily/formatters/md/DayMd.h"
#include "queries/daily/formatters/tex/DayTex.h" //Tex 格式化器头文件
#include "queries/daily/formatters/typ/DayTyp.h" // typ
#include "queries/shared/Interface/IReportFormatter.h" // Ensure this is included
#include "queries/shared/DailyReportData.h" // Ensure this is included



#include <stdexcept>

std::unique_ptr<IReportFormatter<DailyReportData>> DayFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<DayMd>();
        case ReportFormat::LaTeX:
            return std::make_unique<DayTex>();
        case ReportFormat::Typ:
            return std::make_unique<DayTyp>();
        default:
            throw std::invalid_argument("Unsupported report format requested.");
    }
}