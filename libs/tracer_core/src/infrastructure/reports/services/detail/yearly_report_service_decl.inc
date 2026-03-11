class YearlyReportService {
 public:
  explicit YearlyReportService(sqlite3* sqlite_db,
                               const ReportCatalog& report_catalog);

  auto GenerateReports(ReportFormat format) -> FormattedYearlyReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};
