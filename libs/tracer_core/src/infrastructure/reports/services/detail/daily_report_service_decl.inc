class DailyReportService {
 public:
  explicit DailyReportService(sqlite3* sqlite_db,
                              const ReportCatalog& report_catalog);

  auto GenerateAllReports(ReportFormat format) -> FormattedGroupedReports;

 private:
  sqlite3* db_;
  const ReportCatalog& report_catalog_;
};
