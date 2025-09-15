// queries/shared/factories/FmtFactory.hpp
#ifndef REPORT_FMT_FACTORY_HPP
#define REPORT_FMT_FACTORY_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/ReportFormat.hpp"
#include <memory>
#include <stdexcept>

/**
 * @class ReportFmtFactory
 * @brief A generic, templated factory for creating various report formatters.
 * @tparam ReportDataType The data structure the report depends on (e.g., DailyReportData).
 * @tparam MdFormatter    The concrete implementation class for the Markdown formatter.
 * @tparam TexFormatter   The concrete implementation class for the LaTeX formatter.
 */
template<
    typename ReportDataType, 
    typename MdFormatter, 
    typename TexFormatter
>
class ReportFmtFactory {
public:
    /**
     * @brief Creates a formatter instance based on the specified format.
     * @param format The desired report format.
     * @return A smart pointer to the IReportFormatter<ReportDataType> interface.
     */
    static std::unique_ptr<IReportFormatter<ReportDataType>> create_formatter(ReportFormat format) {
        switch (format) {
            case ReportFormat::Markdown:
                return std::make_unique<MdFormatter>();
            case ReportFormat::LaTeX:
                return std::make_unique<TexFormatter>();
            case ReportFormat::Typ:
                // Typst formatter requires special handling due to its config file dependency.
                // It should be created manually in the generator class.
                throw std::invalid_argument("Typst format is not supported by the generic factory.");
            default:
                throw std::invalid_argument("Unsupported report format requested.");
        }
    }
};

#endif // REPORT_FMT_FACTORY_HPP