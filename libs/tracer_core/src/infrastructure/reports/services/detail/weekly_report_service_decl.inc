class WeeklyReportService {
 public:
  explicit WeeklyReportService(sqlite3* database_connection,
                               const ReportCatalog& report_catalog);

  [[nodiscard]] auto GenerateReports(ReportFormat format)
      -> FormattedWeeklyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};
