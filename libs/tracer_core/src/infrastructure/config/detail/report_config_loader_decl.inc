class ReportConfigLoader {
 public:
  static auto LoadDailyTexConfig(const std::filesystem::path& path)
      -> DailyTexConfig;
  static auto LoadDailyTypConfig(const std::filesystem::path& path)
      -> DailyTypConfig;
  static auto LoadDailyMdConfig(const std::filesystem::path& path)
      -> DailyMdConfig;

  static auto LoadMonthlyTexConfig(const std::filesystem::path& path)
      -> MonthlyTexConfig;
  static auto LoadMonthlyTypConfig(const std::filesystem::path& path)
      -> MonthlyTypConfig;
  static auto LoadMonthlyMdConfig(const std::filesystem::path& path)
      -> MonthlyMdConfig;

  static auto LoadPeriodTexConfig(const std::filesystem::path& path)
      -> PeriodTexConfig;
  static auto LoadPeriodTypConfig(const std::filesystem::path& path)
      -> PeriodTypConfig;
  static auto LoadPeriodMdConfig(const std::filesystem::path& path)
      -> PeriodMdConfig;

  static auto LoadWeeklyTexConfig(const std::filesystem::path& path)
      -> WeeklyTexConfig;
  static auto LoadWeeklyTypConfig(const std::filesystem::path& path)
      -> WeeklyTypConfig;
  static auto LoadWeeklyMdConfig(const std::filesystem::path& path)
      -> WeeklyMdConfig;

  static auto LoadYearlyTexConfig(const std::filesystem::path& path)
      -> YearlyTexConfig;
  static auto LoadYearlyTypConfig(const std::filesystem::path& path)
      -> YearlyTypConfig;
  static auto LoadYearlyMdConfig(const std::filesystem::path& path)
      -> YearlyMdConfig;
};
