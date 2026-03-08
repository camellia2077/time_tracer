class InputCollectionStage {
 public:
  static auto Execute(
      PipelineSession& session,
      const tracer_core::application::ports::IIngestInputProvider&
          input_provider,
      const std::string& extension = ".txt") -> bool;
};

class StructureValidationStage {
 public:
  static auto Execute(PipelineSession& session) -> bool;
};

class ConversionStage {
 public:
  static auto Execute(PipelineSession& session) -> bool;

 private:
  static void PrintTiming(double total_time_ms);
};

class CrossMonthLinkStage {
 public:
  static auto Execute(PipelineSession& session) -> bool;
};

class LogicValidationStage {
 public:
  static auto Execute(PipelineSession& session) -> bool;
};
