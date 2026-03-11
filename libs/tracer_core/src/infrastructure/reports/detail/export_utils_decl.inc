struct ReportFormatDetails {
  std::string dir_name;
  std::string extension;
};

[[nodiscard]] auto GetReportFormatDetails(ReportFormat format)
    -> std::optional<ReportFormatDetails>;

void ExecuteExportTask(const std::string& report_type_name_singular,
                       const std::filesystem::path& export_root_path,
                       const std::function<int()>& file_writing_lambda);
