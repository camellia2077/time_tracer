class SqliteDatabaseHealthChecker final
    : public tracer_core::application::ports::IDatabaseHealthChecker {
 public:
  explicit SqliteDatabaseHealthChecker(std::string db_path);

  auto CheckReady()
      -> tracer_core::application::ports::DatabaseHealthCheckResult override;

 private:
  std::string db_path_;
};
