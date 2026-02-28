// application/ports/i_ingest_input_provider.hpp
#ifndef APPLICATION_PORTS_I_INGEST_INPUT_PROVIDER_H_
#define APPLICATION_PORTS_I_INGEST_INPUT_PROVIDER_H_

#include <filesystem>
#include <string_view>

#include "application/dto/ingest_input_model.hpp"

namespace tracer_core::application::ports {

class IIngestInputProvider {
 public:
  virtual ~IIngestInputProvider() = default;

  [[nodiscard]] virtual auto CollectTextInputs(
      const std::filesystem::path& input_root, std::string_view extension) const
      -> tracer_core::application::dto::IngestInputCollection = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_INGEST_INPUT_PROVIDER_H_
