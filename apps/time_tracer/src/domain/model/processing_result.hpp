// domain/model/processing_result.hpp
#ifndef DOMAIN_MODEL_PROCESSING_RESULT_H_
#define DOMAIN_MODEL_PROCESSING_RESULT_H_

struct ProcessingTimings {
  double validation_source_ms = 0.0;
  double conversion_ms = 0.0;
  double validation_output_ms = 0.0;
};

struct ProcessingResult {
  bool success = true;
  ProcessingTimings timings;
};

#endif  // DOMAIN_MODEL_PROCESSING_RESULT_H_
