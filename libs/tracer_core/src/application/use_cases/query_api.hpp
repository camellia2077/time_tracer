#ifndef APPLICATION_USE_CASES_QUERY_API_HPP_
#define APPLICATION_USE_CASES_QUERY_API_HPP_

#include <memory>

#include "application/ports/i_data_query_service.hpp"
#include "application/use_cases/i_query_api.hpp"
#include "domain/repositories/i_project_repository.hpp"

namespace tracer::core::application::use_cases {

class QueryApi final : public IQueryApi {
 public:
  using ProjectRepositoryPtr = std::shared_ptr<IProjectRepository>;
  using DataQueryServicePtr =
      std::shared_ptr<tracer_core::application::ports::IDataQueryService>;

  QueryApi(ProjectRepositoryPtr project_repository,
           DataQueryServicePtr data_query_service);

  auto RunDataQuery(const tracer_core::core::dto::DataQueryRequest& request)
      -> tracer_core::core::dto::TextOutput override;

  auto RunTreeQuery(const tracer_core::core::dto::TreeQueryRequest& request)
      -> tracer_core::core::dto::TreeQueryResponse override;

 private:
  ProjectRepositoryPtr project_repository_;
  DataQueryServicePtr data_query_service_;
};

}  // namespace tracer::core::application::use_cases

using QueryApi = tracer::core::application::use_cases::QueryApi;

#endif  // APPLICATION_USE_CASES_QUERY_API_HPP_
