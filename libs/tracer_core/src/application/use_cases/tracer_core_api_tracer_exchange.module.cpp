#include <exception>
#include <string>
#include <string_view>

#include "application/dto/core_requests.hpp"
#include "application/dto/core_responses.hpp"
#include "application/use_cases/tracer_core_api.hpp"

namespace {

template <typename TResult>
auto BuildTracerExchangeFailure(std::string_view operation,
                                std::string_view detail) -> TResult {
  TResult result{};
  result.ok = false;
  result.error_message =
      std::string(operation) + " failed: " + std::string(detail);
  return result;
}

template <typename TResult>
auto BuildServiceUnavailable(std::string_view operation) -> TResult {
  return BuildTracerExchangeFailure<TResult>(
      operation, "tracer exchange service is not configured.");
}

template <typename TResult>
auto BuildUnexpectedFailure(std::string_view operation) -> TResult {
  return BuildTracerExchangeFailure<TResult>(operation,
                                             "unexpected internal error.");
}

}  // namespace

namespace tracer::core::application::use_cases {

auto TracerCoreApi::RunTracerExchangeExport(
    const tracer_core::core::dto::TracerExchangeExportRequest& request)
    -> tracer_core::core::dto::TracerExchangeExportResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<
          tracer_core::core::dto::TracerExchangeExportResult>(
          "RunTracerExchangeExport");
    }
    return tracer_exchange_service_->RunExport(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<
        tracer_core::core::dto::TracerExchangeExportResult>(
        "RunTracerExchangeExport", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<
        tracer_core::core::dto::TracerExchangeExportResult>(
        "RunTracerExchangeExport");
  }
}

auto TracerCoreApi::RunTracerExchangeImport(
    const tracer_core::core::dto::TracerExchangeImportRequest& request)
    -> tracer_core::core::dto::TracerExchangeImportResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<
          tracer_core::core::dto::TracerExchangeImportResult>(
          "RunTracerExchangeImport");
    }
    return tracer_exchange_service_->RunImport(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<
        tracer_core::core::dto::TracerExchangeImportResult>(
        "RunTracerExchangeImport", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<
        tracer_core::core::dto::TracerExchangeImportResult>(
        "RunTracerExchangeImport");
  }
}

auto TracerCoreApi::RunTracerExchangeInspect(
    const tracer_core::core::dto::TracerExchangeInspectRequest& request)
    -> tracer_core::core::dto::TracerExchangeInspectResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<
          tracer_core::core::dto::TracerExchangeInspectResult>(
          "RunTracerExchangeInspect");
    }
    return tracer_exchange_service_->RunInspect(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<
        tracer_core::core::dto::TracerExchangeInspectResult>(
        "RunTracerExchangeInspect", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<
        tracer_core::core::dto::TracerExchangeInspectResult>(
        "RunTracerExchangeInspect");
  }
}

}  // namespace tracer::core::application::use_cases
