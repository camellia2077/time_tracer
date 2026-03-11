class LazySqliteReportDataQueryService final
    : public tracer_core::application::ports::IReportDataQueryService {
 public:
  LazySqliteReportDataQueryService(
      std::filesystem::path db_path,
      std::shared_ptr<tracer_core::application::ports::IPlatformClock>
          platform_clock);

  auto QueryDaily(std::string_view date) -> DailyReportData override;
  auto QueryMonthly(std::string_view month) -> MonthlyReportData override;
  auto QueryPeriod(int days) -> PeriodReportData override;
  auto QueryRange(std::string_view start_date, std::string_view end_date)
      -> PeriodReportData override;
  auto QueryWeekly(std::string_view iso_week) -> WeeklyReportData override;
  auto QueryYearly(std::string_view year) -> YearlyReportData override;

  auto QueryPeriodBatch(const std::vector<int>& days_list)
      -> std::map<int, PeriodReportData> override;
  auto QueryAllDaily() -> std::map<std::string, DailyReportData> override;
  auto QueryAllMonthly() -> std::map<std::string, MonthlyReportData> override;
  auto QueryAllWeekly() -> std::map<std::string, WeeklyReportData> override;
  auto QueryAllYearly() -> std::map<std::string, YearlyReportData> override;

 private:
  std::filesystem::path db_path_;
  std::shared_ptr<tracer_core::application::ports::IPlatformClock>
      platform_clock_;
};
