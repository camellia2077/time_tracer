import tracer.core.shared;

#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using tracer::core::shared::modtext::Canonicalize;
using tracer::core::shared::modtext::RequireCanonicalText;
using tracer::core::shared::modtext::ToUtf8Bytes;
using tracer::core::shared::modperiod::FormatIsoWeek;
using tracer::core::shared::modperiod::IsoWeek;
using tracer::core::shared::modperiod::IsoWeekEndDate;
using tracer::core::shared::modperiod::IsoWeekStartDate;
using tracer::core::shared::modperiod::ParseGregorianYear;
using tracer::core::shared::modperiod::ParseIsoWeek;
using tracer::core::shared::modtypes::AppError;
using tracer::core::shared::modtypes::AppExitCode;
using tracer::core::shared::modtypes::LogicError;
using tracer::core::shared::string_utils::SplitString;
using tracer::core::shared::string_utils::Trim;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestStringModuleContract(int& failures) {
  Expect(Trim("  hello  ") == "hello", "Trim module contract mismatch.",
         failures);
  const std::vector<std::string> tokens = SplitString("a_b_c", '_');
  Expect(tokens.size() == 3U, "SplitString module contract size mismatch.",
         failures);
  Expect(tokens.size() == 3U && tokens[1] == "b",
         "SplitString module contract content mismatch.", failures);
}

void TestCanonicalTextContract(int& failures) {
  const std::string legacy_text = "\xEF\xBB\xBFy2026\r\nm03\r0101\n";
  Expect(RequireCanonicalText(legacy_text, "legacy.txt") ==
             "y2026\nm03\n0101\n",
         "Canonical text normalization should drop BOM and normalize line endings.",
         failures);

  const auto empty_text = Canonicalize(std::string_view{}, "empty.txt");
  Expect(empty_text.ok && empty_text.text.empty(),
         "Canonicalize should accept empty UTF-8 input.", failures);

  const std::string bom_only = "\xEF\xBB\xBF";
  const auto bom_only_text = Canonicalize(bom_only, "bom-only.txt");
  Expect(bom_only_text.ok && bom_only_text.text.empty(),
         "Canonicalize should accept a BOM-only UTF-8 file.", failures);

  const std::vector<std::uint8_t> invalid_bytes = {0xFFU, 0x61U};
  const auto invalid = Canonicalize(
      std::span<const std::uint8_t>(invalid_bytes.data(), invalid_bytes.size()),
      "invalid.txt");
  Expect(!invalid.ok && invalid.text.empty(),
         "Canonicalize should reject invalid UTF-8 bytes.", failures);
  Expect(!invalid.ok &&
             invalid.error_message.find("Invalid UTF-8") != std::string::npos,
         "Canonicalize invalid UTF-8 error should mention UTF-8 validation.",
         failures);

  Expect(ToUtf8Bytes("a\nb") == std::vector<std::uint8_t>{'a', '\n', 'b'},
         "ToUtf8Bytes should preserve canonical UTF-8 bytes.", failures);
}

void TestPeriodBridge(int& failures) {
  int year = 0;
  Expect(ParseGregorianYear("2025", year), "ParseGregorianYear should pass.",
         failures);
  Expect(year == 2025, "ParseGregorianYear result mismatch.", failures);

  IsoWeek week{};
  Expect(ParseIsoWeek("2025-W08", week), "ParseIsoWeek should pass.", failures);
  Expect(week.year == 2025 && week.week == 8, "ParseIsoWeek result mismatch.",
         failures);
  Expect(FormatIsoWeek(week) == "2025-W08", "FormatIsoWeek mismatch.", failures);
  Expect(!IsoWeekStartDate(week).empty(), "IsoWeekStartDate should not be empty.",
         failures);
  Expect(!IsoWeekEndDate(week).empty(), "IsoWeekEndDate should not be empty.",
         failures);
}

void TestTypesBridge(int& failures) {
  try {
    throw LogicError("logic");
  } catch (const AppError&) {
    // pass
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Exception bridge catch type mismatch.\n";
  }

  Expect(static_cast<int>(AppExitCode::kSuccess) == 0,
         "AppExitCode bridge mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestStringModuleContract(failures);
  TestCanonicalTextContract(failures);
  TestPeriodBridge(failures);
  TestTypesBridge(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_shared_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_shared_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
