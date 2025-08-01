// PeriodFmtFactory.h
#ifndef PERIOD_REPORT_FORMATTER_FACTORY_H
#define PERIOD_REPORT_FORMATTER_FACTORY_H

#include "queries/shared/Interface/IReportFormatter.h"  // 替换 IPeriodFmt.h
#include "queries/shared/PeriodReportData.h"  // 为模板类型引入定义
#include "queries/shared/ReportFormat.h"
#include <memory>

/**
 * @class PeriodFmtFactory
 * @brief 负责创建具体周期报告格式化器实例的工厂。
 */
class PeriodFmtFactory {
public:
    /**
     * @brief 根据指定的格式创建一个格式化器实例。
     * @param format 期望的报告格式。
     * @return 一个指向 IReportFormatter<PeriodReportData> 接口的智能指针。
     */
    static std::unique_ptr<IReportFormatter<PeriodReportData>> create_formatter(ReportFormat format);
};

#endif // PERIOD_REPORT_FORMATTER_FACTORY_H