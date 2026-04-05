#include "api/c_api/capabilities/reporting/tracer_core_c_api_reporting_internal.hpp"

#include <sodium.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "api/c_api/runtime/tracer_core_c_api_internal.hpp"
#include "application/dto/reporting_responses.hpp"
#include "application/dto/shared_envelopes.hpp"
#include "tracer/transport/envelope.hpp"

namespace tt_transport = tracer::transport;

using tracer_core::core::c_api::internal::BuildTextResponse;
using tracer_core::core::dto::TextOutput;

namespace {

auto ToLowerHex(const unsigned char* bytes, size_t size) -> std::string {
  constexpr std::array<char, 16> kHex = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
  };
  constexpr unsigned kHexShiftBits = 4U;
  constexpr std::uint8_t kHexNibbleMask = 0x0FU;
  std::string out;
  out.resize(size * 2U);
  for (size_t index = 0; index < size; ++index) {
    const unsigned char kByte = bytes[index];
    out[index * 2U] = kHex[(kByte >> kHexShiftBits) & kHexNibbleMask];
    out[(index * 2U) + 1U] = kHex[kByte & kHexNibbleMask];
  }
  return out;
}

auto ComputeSha256Hex(std::string_view text) -> std::optional<std::string> {
  std::array<unsigned char, crypto_hash_sha256_BYTES> digest{};
  const auto* input = reinterpret_cast<const unsigned char*>(text.data());
  if (crypto_hash_sha256(digest.data(), input,
                         static_cast<unsigned long long>(text.size())) != 0) {
    return std::nullopt;
  }
  return ToLowerHex(digest.data(), digest.size());
}

}  // namespace

namespace tracer_core::core::c_api::reporting {

auto BuildReportTextResponse(const TextOutput& output) -> const char* {
  if (!output.ok) {
    return BuildTextResponse(output);
  }

  auto envelope = tt_transport::BuildResponseEnvelope(
      output.ok, output.error_message, output.content);
  envelope.report_hash_sha256 = ComputeSha256Hex(output.content);
  if (output.report_window_metadata.has_value()) {
    envelope.report_window_metadata =
        tt_transport::ReportWindowMetadataPayload{
            .has_records = output.report_window_metadata->has_records,
            .matched_day_count =
                output.report_window_metadata->matched_day_count,
            .matched_record_count =
                output.report_window_metadata->matched_record_count,
            .start_date = output.report_window_metadata->start_date,
            .end_date = output.report_window_metadata->end_date,
            .requested_days = output.report_window_metadata->requested_days,
        };
  }
  tracer_core::core::c_api::internal::g_last_response =
      tt_transport::SerializeResponseEnvelope(envelope);
  return tracer_core::core::c_api::internal::g_last_response.c_str();
}

}  // namespace tracer_core::core::c_api::reporting
