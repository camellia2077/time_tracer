class MonthlyReportService {
 public:
  explicit MonthlyReportService(sqlite3* database_connection,
                                const ReportCatalog& report_catalog);

  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedMonthlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};
