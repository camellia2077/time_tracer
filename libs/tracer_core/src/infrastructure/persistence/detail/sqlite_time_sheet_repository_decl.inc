class SqliteTimeSheetRepository final
    : public tracer_core::application::ports::ITimeSheetRepository {
 public:
  explicit SqliteTimeSheetRepository(const std::string& db_path);

  [[nodiscard]] auto IsDbOpen() const -> bool override;
  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records)
      -> void override;
  auto ReplaceMonthData(int year, int month, const std::vector<DayData>& days,
                        const std::vector<TimeRecordInternal>& records)
      -> void override;
  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(std::string_view date)
      const
      -> std::optional<tracer_core::application::ports::PreviousActivityTail>
          override;

 private:
  importer::Repository repository_;
};
