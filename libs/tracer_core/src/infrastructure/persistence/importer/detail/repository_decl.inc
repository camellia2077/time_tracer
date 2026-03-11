/**
 * @class Repository
 * @brief Facade class that simplifies the process of importing data into the
 * database.
 */
class Repository {
 public:
  struct LatestActivityTail {
    std::string date;
    std::string end_time;
  };

  explicit Repository(const std::string& db_path);
  ~Repository();

  [[nodiscard]] auto IsDbOpen() const -> bool;

  auto ImportData(const std::vector<DayData>& days,
                  const std::vector<TimeRecordInternal>& records) -> void;
  auto ReplaceMonthData(int year, int month, const std::vector<DayData>& days,
                        const std::vector<TimeRecordInternal>& records) -> void;
  [[nodiscard]] auto TryGetLatestActivityTailBeforeDate(
      std::string_view date) const -> std::optional<LatestActivityTail>;

 private:
  auto EnsureWriteRepositoryReady() -> void;

  std::string db_path_;
  std::unique_ptr<sqlite::Connection> connection_manager_;
  std::unique_ptr<sqlite::Statement> statement_manager_;
  std::unique_ptr<sqlite::Writer> data_inserter_;
};
