module;

#include <memory>

#include "application/ports/i_ingest_input_provider.hpp"
#include "application/ports/i_processed_data_loader.hpp"
#include "application/ports/i_processed_data_storage.hpp"
#include "infrastructure/io/internal/runtime_adapter_types.hpp"

export module tracer.adapters.io.runtime;

export namespace tracer::adapters::io::modruntime {

using IngestInputProvider = ::tracer_core::application::ports::IIngestInputProvider;
using ProcessedDataLoader = ::tracer_core::application::ports::IProcessedDataLoader;
using ProcessedDataStorage =
    ::tracer_core::application::ports::IProcessedDataStorage;

// Keep runtime assembly on port-facing factories instead of legacy headers.
[[nodiscard]] inline auto CreateTxtIngestInputProvider()
    -> std::shared_ptr<IngestInputProvider> {
  return std::make_shared<
      infrastructure::io::internal::TxtIngestInputProviderAdapter>();
}

[[nodiscard]] inline auto CreateProcessedDataLoader()
    -> std::shared_ptr<ProcessedDataLoader> {
  return std::make_shared<
      infrastructure::io::internal::ProcessedDataLoaderAdapter>();
}

[[nodiscard]] inline auto CreateProcessedDataStorage()
    -> std::shared_ptr<ProcessedDataStorage> {
  return std::make_shared<
      infrastructure::io::internal::ProcessedDataStorageAdapter>();
}

}  // namespace tracer::adapters::io::modruntime
