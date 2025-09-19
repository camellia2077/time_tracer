// queries/shared/factories/TreeFmtFactory.hpp
#ifndef PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP
#define PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP

#include <memory>
#include "queries/shared/interface/ITreeFmt.hpp"
#include "queries/shared/types/ReportFormat.hpp" // The shared enum

class TreeFmtFactory {
public:
    static std::unique_ptr<ITreeFmt> createFormatter(ReportFormat format);
};

#endif // PROJECT_BREAKDOWN_FORMATTER_FACTORY_HPP