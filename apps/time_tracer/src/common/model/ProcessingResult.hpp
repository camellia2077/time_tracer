// common/model/ProcessingResult.hpp
#ifndef PROCESSING_RESULT_HPP
#define PROCESSING_RESULT_HPP

struct ProcessingTimings {
    double validation_source_ms = 0.0;
    double conversion_ms = 0.0;
    double validation_output_ms = 0.0;
};

struct ProcessingResult {
    bool success = true;
    ProcessingTimings timings;
};

#endif // PROCESSING_RESULT_HPP