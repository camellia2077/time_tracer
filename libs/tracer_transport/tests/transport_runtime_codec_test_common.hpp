// tests/transport_runtime_codec_test_common.hpp
#ifndef TRACER_TRANSPORT_TESTS_TRANSPORT_RUNTIME_CODEC_TEST_COMMON_HPP_
#define TRACER_TRANSPORT_TESTS_TRANSPORT_RUNTIME_CODEC_TEST_COMMON_HPP_

#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include "nlohmann/json.hpp"
#include "tracer/transport/runtime_codec.hpp"

namespace tracer_transport_runtime_codec_tests {

using nlohmann::json;
using tracer::transport::CapabilitiesResponsePayload;
using tracer::transport::ConvertRequestPayload;
using tracer::transport::DecodeAckResponse;
using tracer::transport::DecodeConvertRequest;
using tracer::transport::DecodeExportRequest;
using tracer::transport::DecodeImportRequest;
using tracer::transport::DecodeIngestRequest;
using tracer::transport::DecodeQueryRequest;
using tracer::transport::DecodeReportBatchRequest;
using tracer::transport::DecodeReportRequest;
using tracer::transport::DecodeResolveCliContextResponse;
using tracer::transport::DecodeRuntimeCheckResponse;
using tracer::transport::DecodeTextResponse;
using tracer::transport::DecodeTreeRequest;
using tracer::transport::DecodeTreeResponse;
using tracer::transport::DecodeValidateLogicRequest;
using tracer::transport::DecodeValidateStructureRequest;
using tracer::transport::EncodeCapabilitiesResponse;
using tracer::transport::EncodeConvertRequest;
using tracer::transport::EncodeExportRequest;
using tracer::transport::EncodeExportResponse;
using tracer::transport::EncodeImportRequest;
using tracer::transport::EncodeIngestRequest;
using tracer::transport::EncodeIngestResponse;
using tracer::transport::EncodeQueryRequest;
using tracer::transport::EncodeQueryResponse;
using tracer::transport::EncodeReportBatchRequest;
using tracer::transport::EncodeReportBatchResponse;
using tracer::transport::EncodeReportRequest;
using tracer::transport::EncodeReportResponse;
using tracer::transport::EncodeTreeRequest;
using tracer::transport::EncodeTreeResponse;
using tracer::transport::EncodeValidateLogicRequest;
using tracer::transport::EncodeValidateStructureRequest;
using tracer::transport::ExportRequestPayload;
using tracer::transport::ExportResponsePayload;
using tracer::transport::ImportRequestPayload;
using tracer::transport::IngestRequestPayload;
using tracer::transport::IngestResponsePayload;
using tracer::transport::ProjectTreeNodePayload;
using tracer::transport::QueryRequestPayload;
using tracer::transport::QueryResponsePayload;
using tracer::transport::ReportBatchRequestPayload;
using tracer::transport::ReportBatchResponsePayload;
using tracer::transport::ReportRequestPayload;
using tracer::transport::ReportResponsePayload;
using tracer::transport::TreeRequestPayload;
using tracer::transport::TreeResponsePayload;
using tracer::transport::ValidateLogicRequestPayload;
using tracer::transport::ValidateStructureRequestPayload;

inline auto Contains(std::string_view text, std::string_view pattern) -> bool {
  return text.find(pattern) != std::string_view::npos;
}

inline auto Expect(bool condition, std::string_view message, int& failures)
    -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

template <typename Fn>
void ExpectInvalidArgument(Fn&& fn, std::string_view contains_message,
                           std::string_view test_name, int& failures) {
  try {
    fn();
  } catch (const std::invalid_argument& error) {
    if (!Contains(error.what(), contains_message)) {
      ++failures;
      std::cerr << "[FAIL] " << test_name
                << ": unexpected invalid_argument message: " << error.what()
                << '\n';
    }
    return;
  } catch (const std::exception& error) {
    ++failures;
    std::cerr << "[FAIL] " << test_name
              << ": expected invalid_argument, got exception: " << error.what()
              << '\n';
    return;
  }

  ++failures;
  std::cerr << "[FAIL] " << test_name << ": expected invalid_argument.\n";
}

auto RunDecodeRequestTests(int& failures) -> void;
auto RunDecodeResponseTests(int& failures) -> void;
auto RunEncodeTests(int& failures) -> void;

}  // namespace tracer_transport_runtime_codec_tests

#endif  // TRACER_TRANSPORT_TESTS_TRANSPORT_RUNTIME_CODEC_TEST_COMMON_HPP_
