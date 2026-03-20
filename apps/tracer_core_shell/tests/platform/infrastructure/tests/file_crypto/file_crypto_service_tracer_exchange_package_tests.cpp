// infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_package_tests.cpp
#include <exception>
#include <string>

#include "infrastructure/tests/file_crypto/file_crypto_service_tracer_exchange_test_support.hpp"

namespace android_runtime_tests {
namespace {

using namespace file_crypto_tests_internal;
using namespace tracer_exchange_tests_internal;

auto TestTracerExchangePackageRoundTrip(int& failures) -> void {
  const auto payloads = BuildSamplePayloads();
  const auto entries = BuildValidPackageEntries(payloads, "main = true\n",
                                                "alias = true\n",
                                                "duration = true\n");
  const auto bytes = exchange_pkg::EncodePackageBytes(entries);
  const auto decoded = exchange_pkg::DecodePackageBytes(bytes);

  Expect(decoded.manifest.package_type == "tracer_exchange",
         "Decoded manifest package_type should be tracer_exchange.", failures);
  Expect(decoded.manifest.package_version == 3,
         "Decoded manifest package_version should be 3.", failures);
  Expect(decoded.manifest.source_root_name == "data",
         "Decoded manifest should retain source_root_name.", failures);
  Expect(decoded.manifest.payload_files.size() == payloads.size(),
         "Decoded manifest should retain all payload file paths.", failures);
  Expect(decoded.entries.size() ==
             exchange_pkg::kRequiredPackagePaths.size() + payloads.size(),
         "Decoded package should contain fixed entries plus all payload files.",
         failures);

  for (const auto& payload : payloads) {
    const auto* payload_entry = FindEntry(decoded, payload.relative_path);
    Expect(payload_entry != nullptr,
           "Decoded package should contain each payload entry.", failures);
    if (payload_entry != nullptr) {
      const std::string payload_text(payload_entry->data.begin(),
                                     payload_entry->data.end());
      Expect(payload_text == payload.text,
             "Decoded payload bytes should match original payload bytes.",
             failures);
    }
  }
}

auto TestTracerExchangeDecodeRejectsShaMismatch(int& failures) -> void {
  auto bytes = exchange_pkg::EncodePackageBytes(
      BuildValidPackageEntries(BuildSamplePayloads(), "main = true\n",
                               "alias = true\n", "duration = true\n"));
  if (!bytes.empty()) {
    bytes.back() ^= 0x01U;
  }

  bool did_throw = false;
  std::string message;
  try {
    static_cast<void>(exchange_pkg::DecodePackageBytes(bytes));
  } catch (const std::exception& error) {
    did_throw = true;
    message = error.what();
  }

  Expect(did_throw, "DecodePackageBytes should reject SHA-256 mismatches.",
         failures);
  Expect(Contains(message, "entry SHA-256 mismatch"),
         "DecodePackageBytes error should mention SHA-256 mismatch.", failures);
}

auto TestTracerExchangeManifestRejectsPathDrift(int& failures) -> void {
  exchange_pkg::TracerExchangeManifest manifest{};
  manifest.producer_platform = "windows";
  manifest.producer_app = "time_tracer_cli";
  manifest.created_at_utc = "2026-03-18T12:34:56Z";
  manifest.source_root_name = "data";
  manifest.payload_files = {"payload/2025/2025-01.txt"};

  const std::string invalid_manifest = ReplaceFirst(
      exchange_pkg::BuildManifestText(manifest), "payload/2025/2025-01.txt",
      "other/2025-01.txt");

  bool did_throw = false;
  std::string message;
  try {
    static_cast<void>(exchange_pkg::ParseManifestText(invalid_manifest));
  } catch (const std::exception& error) {
    did_throw = true;
    message = error.what();
  }

  Expect(did_throw, "ParseManifestText should reject payload path drift.",
         failures);
  Expect(Contains(message, "manifest payload path must be under `payload/`"),
         "Manifest parse error should mention the payload path contract.",
         failures);
}

}  // namespace

auto RunFileCryptoTracerExchangePackageTests(int& failures) -> void {
  TestTracerExchangePackageRoundTrip(failures);
  TestTracerExchangeDecodeRejectsShaMismatch(failures);
  TestTracerExchangeManifestRejectsPathDrift(failures);
}

}  // namespace android_runtime_tests
