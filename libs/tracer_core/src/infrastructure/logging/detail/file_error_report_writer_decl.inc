class FileErrorReportWriter final
    : public tracer_core::domain::ports::IErrorReportWriter {
 public:
  explicit FileErrorReportWriter(std::filesystem::path file_path);
  FileErrorReportWriter(std::filesystem::path run_file_path,
                        std::filesystem::path latest_file_path);

  auto Append(std::string_view report_content) -> bool override;
  [[nodiscard]] auto DestinationLabel() const -> std::string override;

 private:
  std::filesystem::path run_file_path_;
  std::filesystem::path latest_file_path_;
  bool initialized_ = false;
};
