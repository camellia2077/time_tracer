// query/format/ProjectBreakdownFormatterFactory.cpp (NEW FILE)
#include "ProjectBreakdownFormatterFactory.h"
#include "md/ProjectBreakdownMdFormat.h"
// #include "tex/ProjectBreakdownTexFormat.h" // 未来可以轻松加入新的格式

std::unique_ptr<IProjectBreakdownFormatter> ProjectBreakdownFormatterFactory::createFormatter(ReportFormat format) {
    switch (format) {
        case ReportFormat::Markdown:
            return std::make_unique<ProjectBreakdownMdFormat>();
        // case ReportFormat::LaTeX:
        //     return std::make_unique<ProjectBreakdownTexFormat>();
        default:
            // 默认为 Markdown 或返回空指针
            return std::make_unique<ProjectBreakdownMdFormat>();
    }
}