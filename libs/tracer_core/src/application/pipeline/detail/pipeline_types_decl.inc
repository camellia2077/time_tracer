struct PipelineRunSpec {
  fs::path input_root;
  fs::path output_root;

  DateCheckMode date_check_mode = DateCheckMode::kNone;
  bool structure_validation_blocks_conversion = false;
  bool save_processed_output = false;

  explicit PipelineRunSpec(fs::path out) : output_root(std::move(out)) {}
};

struct PipelineRuntimeState {
  std::vector<tracer_core::application::dto::IngestInputModel> ingest_inputs;
  std::vector<fs::path> generated_files;
  std::shared_ptr<tracer_core::application::ports::IValidationIssueReporter>
      validation_issue_reporter;

  ConverterConfig converter_config;
};

struct PipelineOutput {
  std::map<std::string, std::vector<DailyLog>> processed_data;
};

class PipelineSession {
 public:
  PipelineRunSpec config;
  PipelineRuntimeState state;
  PipelineOutput result;

  explicit PipelineSession(const fs::path& out_root) : config(out_root) {}
};
