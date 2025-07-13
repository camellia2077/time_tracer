#ifndef INTERVAL_CONVERTER_H
#define INTERVAL_CONVERTER_H

#include "InputData.h"
#include "IntervalProcessorConfig.h"

class IntervalConverter {
public:
    explicit IntervalConverter(const IntervalProcessorConfig& config);

    void transform(InputData& day);

private:
    const IntervalProcessorConfig& config_;
};

#endif // INTERVAL_CONVERTER_H