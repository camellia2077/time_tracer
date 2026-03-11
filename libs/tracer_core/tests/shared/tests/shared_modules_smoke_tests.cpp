import tracer.core.shared;

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

using tracer::core::shared::ansi_colors::kRed;
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

void TestStringBridge(int& failures) {
  Expect(Trim("  hello  ") == "hello", "Trim bridge mismatch.", failures);
  const std::vector<std::string> tokens = SplitString("a_b_c", '_');
  Expect(tokens.size() == 3U, "SplitString bridge size mismatch.", failures);
  Expect(tokens.size() == 3U && tokens[1] == "b",
         "SplitString bridge content mismatch.", failures);
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
  Expect(!kRed.empty(), "ANSI colors bridge should expose constants.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestStringBridge(failures);
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
