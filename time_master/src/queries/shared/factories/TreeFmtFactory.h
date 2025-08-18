// query/format/TreeFmtFactory.h (NEW FILE)
#ifndef PROJECT_BREAKDOWN_FORMATTER_FACTORY_H
#define PROJECT_BREAKDOWN_FORMATTER_FACTORY_H

#include <memory>
#include "queries/shared/Interface/ITreeFmt.h"
#include "queries/shared/ReportFormat.h" // The shared enum

class TreeFmtFactory {
public:
    static std::unique_ptr<ITreeFmt> createFormatter(ReportFormat format);
};

#endif // PROJECT_BREAKDOWN_FORMATTER_FACTORY_H