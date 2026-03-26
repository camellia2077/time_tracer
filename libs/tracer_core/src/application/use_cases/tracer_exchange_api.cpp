#include "application/use_cases/tracer_exchange_api.hpp"

#include <exception>
#include <string>
#include <string_view>

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

using tracer_core::core::dto::TracerExchangeExportRequest;
using tracer_core::core::dto::TracerExchangeExportResult;
using tracer_core::core::dto::TracerExchangeImportRequest;
using tracer_core::core::dto::TracerExchangeImportResult;
using tracer_core::core::dto::TracerExchangeInspectRequest;
using tracer_core::core::dto::TracerExchangeInspectResult;

TracerExchangeApi::TracerExchangeApi(
    TracerExchangeServicePtr tracer_exchange_service)
    : tracer_exchange_service_(std::move(tracer_exchange_service)) {}

auto TracerExchangeApi::RunTracerExchangeExport(
    const TracerExchangeExportRequest& request) -> TracerExchangeExportResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeExportResult>(
          "RunTracerExchangeExport");
    }
    return tracer_exchange_service_->RunExport(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeExportResult>(
        "RunTracerExchangeExport", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeExportResult>(
        "RunTracerExchangeExport");
  }
}

auto TracerExchangeApi::RunTracerExchangeImport(
    const TracerExchangeImportRequest& request) -> TracerExchangeImportResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeImportResult>(
          "RunTracerExchangeImport");
    }
    return tracer_exchange_service_->RunImport(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeImportResult>(
        "RunTracerExchangeImport", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeImportResult>(
        "RunTracerExchangeImport");
  }
}

auto TracerExchangeApi::RunTracerExchangeInspect(
    const TracerExchangeInspectRequest& request)
    -> TracerExchangeInspectResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeInspectResult>(
          "RunTracerExchangeInspect");
    }
    return tracer_exchange_service_->RunInspect(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeInspectResult>(
        "RunTracerExchangeInspect", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeInspectResult>(
        "RunTracerExchangeInspect");
  }
}

}  // namespace tracer::core::application::use_cases
