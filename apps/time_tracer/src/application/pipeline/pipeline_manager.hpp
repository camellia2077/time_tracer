// application/pipeline/pipeline_manager.hpp
#ifndef APPLICATION_PIPELINE_PIPELINE_MANAGER_H_
#define APPLICATION_PIPELINE_PIPELINE_MANAGER_H_

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "application/pipeline/context/pipeline_context.hpp"
#include "domain/types/app_options.hpp"

namespace fs = std::filesystem;

namespace time_tracer::application::ports {
class IConverterConfigProvider;
class IIngestInputProvider;
class IProcessedDataStorage;
}  // namespace time_tracer::application::ports

namespace core::pipeline {

class PipelineManager {
 public:
  PipelineManager(
      fs::path output_root,
      std::shared_ptr<time_tracer::application::ports::IConverterConfigProvider>
          converter_config_provider,
      std::shared_ptr<time_tracer::application::ports::IIngestInputProvider>
          ingest_input_provider,
      std::shared_ptr<time_tracer::application::ports::IProcessedDataStorage>
          processed_data_storage);

  [[nodiscard]] auto Run(const std::string& input_path,
                         const AppOptions& options)
      -> std::optional<PipelineContext>;

 private:
  fs::path output_root_;
  std::shared_ptr<time_tracer::application::ports::IConverterConfigProvider>
      converter_config_provider_;
  std::shared_ptr<time_tracer::application::ports::IIngestInputProvider>
      ingest_input_provider_;
  std::shared_ptr<time_tracer::application::ports::IProcessedDataStorage>
      processed_data_storage_;
};

}  // namespace core::pipeline

#endif  // APPLICATION_PIPELINE_PIPELINE_MANAGER_H_
