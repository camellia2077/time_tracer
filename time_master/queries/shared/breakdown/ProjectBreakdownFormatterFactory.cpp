// query/format/ProjectBreakdownFormatterFactory.cpp 
#include "ProjectBreakdownFormatterFactory.h"
#include "queries/shared/breakdown/formatters/BreakdownMd.h"
#include "queries/shared/breakdown/formatters/BreakdownTex.h" 

std::unique_ptr<IProjectBreakdownFormatter> ProjectBreakdownFormatterFactory::createFormatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<BreakdownMd>();
        case ReportFormat::LaTeX:
            return std::make_unique<BreakdownTex>();
        default:
            // 默认为 Markdown 或返回空指针
            return std::make_unique<BreakdownMd>();
    }
}