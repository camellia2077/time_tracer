
// query/format/TreeFmtFactory.cpp 
#include "TreeFmtFactory.h"
#include "queries/shared/formatters/breakdown/md/BreakdownMd.h"
#include "queries/shared/formatters/breakdown/tex/BreakdownTex.h" 
#include "queries/shared/formatters/breakdown/typ/BreakdownTyp.h"

std::unique_ptr<ITreeFmt> TreeFmtFactory::createFormatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<BreakdownMd>();
        case ReportFormat::LaTeX:
            return std::make_unique<BreakdownTex>();
        case ReportFormat::Typ:
            return std::make_unique<BreakdownTyp>();
        default:
            // 默认返回格式为 Markdown 
            return std::make_unique<BreakdownMd>();
    }
}