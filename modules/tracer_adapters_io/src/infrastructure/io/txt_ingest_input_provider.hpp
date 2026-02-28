// infrastructure/io/txt_ingest_input_provider.hpp
#ifndef INFRASTRUCTURE_IO_TXT_INGEST_INPUT_PROVIDER_H_
#define INFRASTRUCTURE_IO_TXT_INGEST_INPUT_PROVIDER_H_

#include <filesystem>
#include <string_view>

#include "application/ports/i_ingest_input_provider.hpp"

namespace infrastructure::io {

class TxtIngestInputProvider final
    : public tracer_core::application::ports::IIngestInputProvider {
 public:
  [[nodiscard]] auto CollectTextInputs(const std::filesystem::path& input_root,
                                       std::string_view extension) const
      -> tracer_core::application::dto::IngestInputCollection override;
};

}  // namespace infrastructure::io

#endif  // INFRASTRUCTURE_IO_TXT_INGEST_INPUT_PROVIDER_H_
