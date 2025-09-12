#ifndef REPORT_FMT_FACTORY_HPP
#define REPORT_FMT_FACTORY_HPP

#include "queries/shared/Interface/IReportFormatter.hpp"
#include "queries/shared/ReportFormat.hpp"
#include <memory>
#include <stdexcept>

/**
 * @class ReportFmtFactory
 * @brief 一个通用的、模板化的工厂，用于创建各种类型的报告格式化器。
 * * @tparam ReportDataType 报告所依赖的数据结构 (例如 DailyReportData)。
 * @tparam MdFormatter    Markdown 格式化器的具体实现类。
 * @tparam TexFormatter   LaTeX 格式化器的具体实现类。
 * @tparam TypFormatter   Typst 格式化器的具体实现类。
 */
template<
    typename ReportDataType, 
    typename MdFormatter, 
    typename TexFormatter, 
    typename TypFormatter
>
class ReportFmtFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter<ReportDataType> 接口的智能指针。
     */
    static std::unique_ptr<IReportFormatter<ReportDataType>> create_formatter(ReportFormat format) {
        switch (format) {
            case ReportFormat::Markdown:
                return std::make_unique<MdFormatter>();
            case ReportFormat::LaTeX:
                return std::make_unique<TexFormatter>();
            case ReportFormat::Typ:
                return std::make_unique<TypFormatter>();
            default:
                throw std::invalid_argument("Unsupported report format requested.");
        }
    }
};

#endif // REPORT_FMT_FACTORY_HPP