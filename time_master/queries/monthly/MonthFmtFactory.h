// queries/report_generators/monthly/formatter/MonthFmtFactory.h
#ifndef MONTHLY_REPORT_FORMATTER_FACTORY_H
#define MONTHLY_REPORT_FORMATTER_FACTORY_H

#include "queries/shared/Interface/IReportFormatter.h"  // 替换 IMonthFmt.h
#include "queries/shared/MonthlyReportData.h" // 为模板类型引入定义
#include "queries/shared/ReportFormat.h"
#include <memory>

/**
 * @class MonthFmtFactory
 * @brief 负责创建具体月报格式化器实例的工厂。
 */
class MonthFmtFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter<MonthlyReportData> 接口的智能指针。
     */
    static std::unique_ptr<IReportFormatter<MonthlyReportData>> create_formatter(ReportFormat format);
};

#endif // MONTHLY_REPORT_FORMATTER_FACTORY_H