// query/format/ProjectBreakdownFormatterFactory.h (NEW FILE)
#ifndef PROJECT_BREAKDOWN_FORMATTER_FACTORY_H
#define PROJECT_BREAKDOWN_FORMATTER_FACTORY_H

#include <memory>
#include "IProjectBreakdownFormatter.h"
#include "report_generators/_shared/ReportFormat.h" // The shared enum

class ProjectBreakdownFormatterFactory {
public:
    static std::unique_ptr<IProjectBreakdownFormatter> createFormatter(ReportFormat format);
};

#endif // PROJECT_BREAKDOWN_FORMATTER_FACTORY_H