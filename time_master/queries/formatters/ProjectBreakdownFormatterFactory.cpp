// query/format/ProjectBreakdownFormatterFactory.cpp 
#include "ProjectBreakdownFormatterFactory.h"
#include "queries/formatters/markdown/ProjectBreakdownMdFormat.h"
#include "queries/formatters/tex/ProjectBreakdownTexFormat.h" 

std::unique_ptr<IProjectBreakdownFormatter> ProjectBreakdownFormatterFactory::createFormatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<ProjectBreakdownMdFormat>();
        case ReportFormat::LaTeX:
            return std::make_unique<ProjectBreakdownTexFormat>();
        default:
            // 默认为 Markdown 或返回空指针
            return std::make_unique<ProjectBreakdownMdFormat>();
    }
}