// generator/api/LogGeneratorFactory.hpp
#ifndef LOG_GENERATOR_FACTORY_HPP
#define LOG_GENERATOR_FACTORY_HPP

#include "ILogGenerator.hpp"
#include "common/AppContext.hpp"
#include "generator/impl/LogGenerator.hpp"
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

#endif // LOG_GENERATOR_FACTORY_HPP