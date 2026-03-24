// application/ports/query/i_data_query_service.hpp
#ifndef APPLICATION_PORTS_I_DATA_QUERY_SERVICE_H_
#define APPLICATION_PORTS_I_DATA_QUERY_SERVICE_H_

#include "application/dto/query_requests.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer_core::application::ports {

class IDataQueryService {
 public:
  virtual ~IDataQueryService() = default;

  virtual auto RunDataQuery(
      const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;
};

}  // namespace tracer_core::application::ports

#endif  // APPLICATION_PORTS_I_DATA_QUERY_SERVICE_H_
