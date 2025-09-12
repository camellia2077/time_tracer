// query/format/TreeFmtFactory.h (NEW FILE)
#ifndef PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP
#define PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP

#include <memory>
#include "queries/shared/Interface/ITreeFmt.hpp"
#include "queries/shared/ReportFormat.hpp" // The shared enum

class TreeFmtFactory {
public:
    static std::unique_ptr<ITreeFmt> createFormatter(ReportFormat format);
};

#endif // PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP