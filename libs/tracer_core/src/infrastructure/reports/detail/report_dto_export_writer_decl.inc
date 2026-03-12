[[nodiscard]] auto CreateReportDtoFormatter(const ReportCatalog& report_catalog)
    -> std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>;

class ReportDtoExportWriter final
    : public tracer_core::application::ports::IReportExportWriter {
 public:
  ReportDtoExportWriter(
      std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
          formatter,
      std::shared_ptr<IReportExporter> exporter);

  auto ExportSingleDay(const std::string& date, const DailyReportData& report,
                       ReportFormat format) -> void override;
  auto ExportSingleMonth(const std::string& month,
                         const MonthlyReportData& report, ReportFormat format)
      -> void override;
  auto ExportSinglePeriod(int days, const PeriodReportData& report,
                          ReportFormat format) -> void override;
  auto ExportSingleWeek(const std::string& iso_week,
                        const WeeklyReportData& report, ReportFormat format)
      -> void override;
  auto ExportSingleYear(const std::string& year, const YearlyReportData& report,
                        ReportFormat format) -> void override;

  auto ExportAllDaily(const std::map<std::string, DailyReportData>& reports,
                      ReportFormat format) -> void override;
  auto ExportAllMonthly(const std::map<std::string, MonthlyReportData>& reports,
                        ReportFormat format) -> void override;
  auto ExportAllPeriod(const std::map<int, PeriodReportData>& reports,
                       ReportFormat format) -> void override;
  auto ExportAllWeekly(const std::map<std::string, WeeklyReportData>& reports,
                       ReportFormat format) -> void override;
  auto ExportAllYearly(const std::map<std::string, YearlyReportData>& reports,
                       ReportFormat format) -> void override;

 private:
  std::shared_ptr<tracer_core::application::ports::IReportDtoFormatter>
      formatter_;
  std::shared_ptr<IReportExporter> exporter_;
};
