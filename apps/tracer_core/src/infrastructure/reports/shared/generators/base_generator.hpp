// infrastructure/reports/shared/generators/base_generator.hpp
#ifndef INFRASTRUCTURE_REPORTS_SHARED_GENERATORS_BASE_GENERATOR_H_
#define INFRASTRUCTURE_REPORTS_SHARED_GENERATORS_BASE_GENERATOR_H_

#include <memory>
#include <string>
// [修复] 更新包含路径
#include "domain/reports/types/report_types.hpp"
#include "infrastructure/config/models/report_catalog.hpp"
#include "infrastructure/reports/shared/factories/generic_formatter_factory.hpp"

/**
 * @class BaseGenerator
 * @brief (模板基类) 为所有报告生成器提供通用的报告生成流程。
 *
 * 使用了模板方法模式，将 "查询 -> 格式化" 的固定流程封装起来。
 * 子类通过指定模板参数来定义具体使用的数据类型、查询器和查询参数。
 *
 * @tparam ReportDataType 报告所依赖的数据结构 (例如 DailyReportData)。
 * @tparam QuerierType 用于获取数据的具体查询器类 (例如 DayQuerier)。
 * @tparam QueryParamType 查询器所接受的参数类型 (例如 const std::string& 或
 * int)。
 */
template <typename ReportDataType, typename QuerierType,
          typename QueryParamType>
class BaseGenerator {
 public:
  /**
   * @brief 构造函数。
   * @param database_connection 指向 SQLite
   * 数据库连接的指针。
   * @param config
   * 应用程序配置对象的常量引用。

   */
  BaseGenerator(sqlite3* database_connection, const ReportCatalog& catalog)
      : db_(database_connection), report_catalog_(catalog) {}

  virtual ~BaseGenerator() = default;

  /**
   * @brief 生成格式化的报告。
   * @param param 用于查询的参数 (例如日期字符串或天数)。
   * @param format 需要生成的报告格式。
   * @return 包含格式化报告的字符串。
   */
  [[nodiscard]] auto GenerateReport(QueryParamType param,
                                    ReportFormat format) const -> std::string {
    // 1. 创建具体的查询器并获取数据
    QuerierType querier(db_, param);
    ReportDataType report_data = querier.FetchData();

    // 2. 使用通用工厂创建格式化器
    auto formatter = GenericFormatterFactory<ReportDataType>::Create(
        format, report_catalog_);

    // 3. 格式化并返回报告
    return formatter->FormatReport(report_data);
  }

 protected:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};

#endif  // INFRASTRUCTURE_REPORTS_SHARED_GENERATORS_BASE_GENERATOR_H_
