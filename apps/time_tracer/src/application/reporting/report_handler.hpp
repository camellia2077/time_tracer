// application/reporting/report_handler.hpp
#ifndef APPLICATION_REPORTING_REPORT_HANDLER_H_
#define APPLICATION_REPORTING_REPORT_HANDLER_H_

#include <memory>

#include "application/interfaces/i_report_handler.hpp"

class ReportGenerator;
class Exporter;

class ReportHandler : public IReportHandler {
 public:
  ReportHandler(std::unique_ptr<ReportGenerator> generator,
                std::unique_ptr<Exporter> exporter);
  ~ReportHandler() override;

  // 查询方法
  std::string run_daily_query(const std::string& date,
                              ReportFormat format) override;
  std::string run_monthly_query(const std::string& month,
                                ReportFormat format) override;
  std::string run_period_query(int days, ReportFormat format) override;

  // [新增] 批量周期查询实现
  std::string run_period_queries(const std::vector<int>& days_list,
                                 ReportFormat format) override;

  // 导出方法
  void run_export_single_day_report(const std::string& date,
                                    ReportFormat format) override;
  void run_export_single_month_report(const std::string& month,
                                      ReportFormat format) override;
  void run_export_single_period_report(int days, ReportFormat format) override;
  void run_export_all_daily_reports_query(ReportFormat format) override;
  void run_export_all_monthly_reports_query(ReportFormat format) override;
  void run_export_all_period_reports_query(const std::vector<int>& days_list,
                                           ReportFormat format) override;

 private:
  std::unique_ptr<ReportGenerator> generator_;
  std::unique_ptr<Exporter> exporter_;
};

#endif
