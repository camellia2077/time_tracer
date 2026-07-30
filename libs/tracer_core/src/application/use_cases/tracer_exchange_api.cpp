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
using tracer_core::core::dto::TracerExchangeContentRequest;
using tracer_core::core::dto::TracerExchangeContentResult;
using tracer_core::core::dto::TracerExchangeExportContent;
using tracer_core::core::dto::TracerExchangeContentEncodingResult;
using tracer_core::core::dto::TracerExchangeImportRequest;
using tracer_core::core::dto::TracerExchangeImportResult;
using tracer_core::core::dto::TracerExchangeInspectRequest;
using tracer_core::core::dto::TracerExchangeInspectResult;
using tracer_core::core::dto::TracerExchangeUnpackRequest;
using tracer_core::core::dto::TracerExchangeUnpackResult;

TracerExchangeApi::TracerExchangeApi(
    TracerExchangeServicePtr tracer_exchange_service)
    : tracer_exchange_service_(std::move(tracer_exchange_service)) {}

auto TracerExchangeApi::BuildTracerExchangeExportContent(
    const TracerExchangeContentRequest& request)
    -> TracerExchangeContentResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeContentResult>(
          "BuildTracerExchangeExportContent");
    }
    return tracer_exchange_service_->BuildExportContent(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeContentResult>(
        "BuildTracerExchangeExportContent", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeContentResult>(
        "BuildTracerExchangeExportContent");
  }
}

auto TracerExchangeApi::EncodeTracerExchangeExportContent(
    const TracerExchangeExportContent& content)
    -> TracerExchangeContentEncodingResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeContentEncodingResult>(
          "EncodeTracerExchangeExportContent");
    }
    return tracer_exchange_service_->EncodeExportContent(content);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeContentEncodingResult>(
        "EncodeTracerExchangeExportContent", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeContentEncodingResult>(
        "EncodeTracerExchangeExportContent");
  }
}

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

auto TracerExchangeApi::RunTracerExchangeUnpack(
    const TracerExchangeUnpackRequest& request) -> TracerExchangeUnpackResult {
  try {
    if (!tracer_exchange_service_) {
      return BuildServiceUnavailable<TracerExchangeUnpackResult>(
          "RunTracerExchangeUnpack");
    }
    return tracer_exchange_service_->RunUnpack(request);
  } catch (const std::exception& exception) {
    return BuildTracerExchangeFailure<TracerExchangeUnpackResult>(
        "RunTracerExchangeUnpack", exception.what());
  } catch (...) {
    return BuildUnexpectedFailure<TracerExchangeUnpackResult>(
        "RunTracerExchangeUnpack");
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
