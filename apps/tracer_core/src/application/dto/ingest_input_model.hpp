// application/dto/ingest_input_model.hpp
#ifndef APPLICATION_DTO_INGEST_INPUT_MODEL_H_
#define APPLICATION_DTO_INGEST_INPUT_MODEL_H_

#include <string>
#include <vector>

namespace tracer_core::application::dto {

struct IngestInputModel {
  std::string source_id;
  std::string source_label;
  std::string content;
};

struct IngestInputCollection {
  bool input_exists = false;
  std::vector<IngestInputModel> inputs;
};

}  // namespace tracer_core::application::dto

#endif  // APPLICATION_DTO_INGEST_INPUT_MODEL_H_
