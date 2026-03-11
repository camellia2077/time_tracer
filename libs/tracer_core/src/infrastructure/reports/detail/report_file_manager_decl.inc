class ReportFileManager {
 public:
  explicit ReportFileManager(std::filesystem::path export_root);

  [[nodiscard]] auto GetSingleDayReportPath(const std::string& date,
                                            ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetSingleMonthReportPath(const std::string& month,
                                              ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetSinglePeriodReportPath(int days,
                                               ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetSingleWeekReportPath(const std::string& iso_week,
                                             ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetSingleYearReportPath(const std::string& year,
                                             ReportFormat format) const
      -> std::filesystem::path;

  [[nodiscard]] auto GetAllDailyReportsBaseDir(ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetAllMonthlyReportsBaseDir(ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetAllPeriodReportsBaseDir(ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetAllWeeklyReportsBaseDir(ReportFormat format) const
      -> std::filesystem::path;
  [[nodiscard]] auto GetAllYearlyReportsBaseDir(ReportFormat format) const
      -> std::filesystem::path;

 private:
  std::filesystem::path export_root_path_;
};
