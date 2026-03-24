#ifndef APPLICATION_USE_CASES_I_QUERY_API_HPP_
#define APPLICATION_USE_CASES_I_QUERY_API_HPP_

#include "application/dto/query_requests.hpp"
#include "application/dto/query_responses.hpp"
#include "application/dto/shared_envelopes.hpp"

namespace tracer::core::application::use_cases {

class IQueryApi {
 public:
  virtual ~IQueryApi() = default;

  virtual auto RunDataQuery(
      const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput = 0;

  virtual auto RunTreeQuery(
      const tracer_core::core::dto::TreeQueryRequest& request)
      -> tracer_core::core::dto::TreeQueryResponse = 0;
};

}  // namespace tracer::core::application::use_cases

using IQueryApi = tracer::core::application::use_cases::IQueryApi;

#endif  // APPLICATION_USE_CASES_I_QUERY_API_HPP_
