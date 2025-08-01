#include "common/pch.h"
// PeriodFmtFactory.cpp
#include "PeriodFmtFactory.h"
#include "queries/period/formatters/md/PeriodMd.h"
#include "queries/period/formatters/tex/PeriodTex.h"
#include "queries/period/formatters/typ/PeriodTyp.h"

#include <stdexcept>

// create_formatter 方法的实现，返回类型已更新
std::unique_ptr<IReportFormatter<PeriodReportData>> PeriodFmtFactory::create_formatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<PeriodMd>();
        case ReportFormat::LaTeX:
            return std::make_unique<PeriodTex>();
        case ReportFormat::Typ:
            return std::make_unique<PeriodTyp>();
        default:
            throw std::invalid_argument("Unsupported report format requested for period report.");
    }
}