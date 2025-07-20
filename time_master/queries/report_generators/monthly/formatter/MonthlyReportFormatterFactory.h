#ifndef MONTHLY_REPORT_FORMATTER_FACTORY_H
#define MONTHLY_REPORT_FORMATTER_FACTORY_H

#include "IReportFormatter.h"
#include "report_generators/_shared/ReportFormat.h"
#include <memory>

/**
 * @class MonthlyReportFormatterFactory
 * @brief 负责创建具体月报格式化器实例的工厂。
 */
class MonthlyReportFormatterFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter 接口的智能指针。
     */
    static std::unique_ptr<IReportFormatter> create_formatter(ReportFormat format);
};

#endif // MONTHLY_REPORT_FORMATTER_FACTORY_H