import tracer.core.infrastructure.exchange;

#include "infra/tests/modules_smoke/exchange.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

namespace exchange = tracer::core::infrastructure::crypto::exchange;

auto BuildEntry(std::string_view path, std::string_view text)
    -> exchange::TracerExchangePackageEntry {
  exchange::TracerExchangePackageEntry entry{};
  entry.relative_path = std::string(path);
  entry.data.assign(text.begin(), text.end());
  entry.entry_flags = exchange::kStandardEntryFlags;
  return entry;
}

}  // namespace

auto RunInfrastructureModuleCryptoExchangeSmoke() -> int {
  exchange::TracerExchangeManifest manifest{};
  manifest.producer_platform = "windows";
  manifest.producer_app = "time_tracer_cli";
  manifest.created_at_utc = "2026-03-18T12:34:56Z";
  manifest.source_root_name = "data";
  manifest.config_files = {
      "config/user/activity_hierarchy/default.toml",
      "config/user/behavior.toml",
      "config/user/notes.bin",
  };
  manifest.payload_files = {
      "payload/2025/2025-01.txt",
      "payload/2026/2026-12.txt",
  };

  const std::string manifest_text = exchange::BuildManifestText(manifest);
  const exchange::TracerExchangeManifest parsed_manifest =
      exchange::ParseManifestText(manifest_text);
  if (parsed_manifest.package_type != "tracer_exchange" ||
      parsed_manifest.package_version != 6 ||
      parsed_manifest.producer_platform != "windows" ||
      parsed_manifest.producer_app != "time_tracer_cli" ||
      parsed_manifest.source_root_name != "data" ||
      parsed_manifest.payload_files.size() != 2U) {
    return 11;
  }

  std::vector<exchange::TracerExchangePackageEntry> entries;
  entries.reserve(exchange::kRequiredPackagePaths.size() + 3U + 2U);
  entries.push_back(BuildEntry(exchange::kManifestPath, manifest_text));
  entries.push_back(BuildEntry("config/user/activity_hierarchy/default.toml",
                               "parent = \"study\"\n\n[canonical]\n\"study\" = "
                               "\"math\"\n"));
  entries.push_back(BuildEntry("config/user/behavior.toml", "main = true\n"));
  auto binary_config = BuildEntry("config/user/notes.bin", "binary\0data");
  binary_config.entry_flags = exchange::kEntryFlagRequired;
  entries.push_back(std::move(binary_config));
  entries.push_back(BuildEntry("payload/2025/2025-01.txt",
                               "y2025\nm01\nd0101\n0600 study_math // alpha\n"));
  entries.push_back(BuildEntry("payload/2026/2026-12.txt",
                               "y2026\nm12\nd1201\n0600 study_math // alpha\n"));

  const auto package_bytes = exchange::EncodePackageBytes(entries);
  const exchange::DecodedTracerExchangePackage decoded =
      exchange::DecodePackageBytes(package_bytes);
  if (decoded.entries.size() !=
      exchange::kRequiredPackagePaths.size() + 3U + 2U) {
    return 12;
  }
  if (decoded.entries.front().relative_path != exchange::kManifestPath) {
    return 13;
  }
  if (decoded.entries.back().relative_path != "payload/2026/2026-12.txt") {
    return 14;
  }

  if (decoded.entries[3].relative_path != "config/user/notes.bin" ||
      decoded.entries[3].entry_flags != exchange::kEntryFlagRequired) {
    return 16;
  }

  const std::string payload(decoded.entries.back().data.begin(),
                            decoded.entries.back().data.end());
  if (payload.find("study_math") == std::string::npos) {
    return 15;
  }
  return 0;
}
