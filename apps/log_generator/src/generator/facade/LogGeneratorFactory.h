#ifndef GENERATOR_FACADE_LOGGENERATORFACTORY_H
#define GENERATOR_FACADE_LOGGENERATORFACTORY_H

#include "ILogGenerator.h"
#include "common/AppContext.h"
#include "LogGenerator.h"
#include <memory>

class ILogGeneratorFactory {
public:
    virtual ~ILogGeneratorFactory() = default;
    virtual std::unique_ptr<ILogGenerator> create(const Core::AppContext& context) = 0;
};

class LogGeneratorFactory : public ILogGeneratorFactory {
public:
    std::unique_ptr<ILogGenerator> create(const Core::AppContext& context) override {
        return std::make_unique<LogGenerator>(
            context.config,
            context.all_activities,
            context.remarks,
            context.activity_remarks,
            context.wake_keywords
        );
    }
};

#endif // GENERATOR_FACADE_LOGGENERATORFACTORY_H