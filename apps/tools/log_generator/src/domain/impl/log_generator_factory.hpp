// domain/impl/log_generator_factory.hpp
#ifndef DOMAIN_IMPL_LOG_GENERATOR_FACTORY_H_
#define DOMAIN_IMPL_LOG_GENERATOR_FACTORY_H_

#include <memory>

#include "application/ports/log_generator_factory.hpp"
#include "domain/impl/log_generator.hpp"

class LogGeneratorFactory : public ILogGeneratorFactory {
 public:
  std::unique_ptr<ILogGenerator> create(
      const Core::AppContext& context) override {
    return std::make_unique<LogGenerator>(
        context.config, context.all_activities, context.remarks,
        context.activity_remarks, context.wake_keywords);
  }
};

#endif  // DOMAIN_IMPL_LOG_GENERATOR_FACTORY_H_
