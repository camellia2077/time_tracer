#include "shared/types/ansi_colors.hpp"
#include "shared/types/exceptions.hpp"
#include "shared/types/exit_codes.hpp"
#include "shared/utils/period_utils.hpp"
#include "shared/utils/string_utils.hpp"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestStringHeaderCompat(int& failures) {
  Expect(Trim("  keep-header-path  ") == "keep-header-path",
         "Legacy Trim header path mismatch.", failures);
  const std::vector<std::string> tokens = SplitString("x_y", '_');
  Expect(tokens.size() == 2U, "Legacy SplitString header path mismatch.",
         failures);
}

void TestPeriodHeaderCompat(int& failures) {
  int year = 0;
  Expect(ParseGregorianYear("2030", year),
         "Legacy ParseGregorianYear header path mismatch.", failures);

  IsoWeek week{};
  Expect(ParseIsoWeek("2030-W10", week),
         "Legacy ParseIsoWeek header path mismatch.", failures);
  Expect(!IsoWeekStartDate(week).empty(),
         "Legacy IsoWeekStartDate header path mismatch.", failures);
}

void TestTypesHeaderCompat(int& failures) {
  try {
    throw tracer_core::common::IoError("io");
  } catch (const tracer_core::common::AppError&) {
    // pass
  } catch (...) {
    ++failures;
    std::cerr << "[FAIL] Legacy exceptions header path mismatch.\n";
  }

  Expect(static_cast<int>(AppExitCode::kSuccess) == 0,
         "Legacy AppExitCode header path mismatch.", failures);
  Expect(!tracer_core::common::colors::kGreen.empty(),
         "Legacy ANSI colors header path mismatch.", failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestStringHeaderCompat(failures);
  TestPeriodHeaderCompat(failures);
  TestTypesHeaderCompat(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_core_shared_legacy_headers_compat_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_core_shared_legacy_headers_compat_tests failures: "
            << failures << '\n';
  return 1;
}
