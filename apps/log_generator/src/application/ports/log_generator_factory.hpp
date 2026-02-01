// application/ports/log_generator_factory.hpp
#ifndef APPLICATION_PORTS_LOG_GENERATOR_FACTORY_H_
#define APPLICATION_PORTS_LOG_GENERATOR_FACTORY_H_

#include <memory>

#include "common/app_context.hpp"
#include "domain/api/i_log_generator.hpp"

class ILogGeneratorFactory {
 public:
  virtual ~ILogGeneratorFactory() = default;
  virtual std::unique_ptr<ILogGenerator> create(
      const Core::AppContext& context) = 0;
};

#endif  // APPLICATION_PORTS_LOG_GENERATOR_FACTORY_H_
