#include "application/tests/modules/pipeline_tests.hpp"

#include <array>

#include "application/pipeline/detail/pipeline_record_time_order_support.hpp"

namespace tracer_core::application::tests {

namespace {

auto TestLogicalDayModeCrossMidnight(TestState& state) -> void {
  using tracer::core::application::pipeline::record_time_order::IsStrictlyAfter;

  Expect(state,
         IsStrictlyAfter("0009", "2058", TimeOrderMode::kLogicalDay0600),
         "logical_day_0600 should treat 0009 as later than 2058.");
  Expect(state,
         !IsStrictlyAfter("0009", "2058", TimeOrderMode::kStrictCalendar),
         "strict_calendar should treat 0009 as earlier than 2058.");

  Expect(state,
         IsStrictlyAfter("0200", "2207", TimeOrderMode::kLogicalDay0600),
         "logical_day_0600 should treat 0200 as later than 2207.");
  Expect(state,
         !IsStrictlyAfter("0200", "2207", TimeOrderMode::kStrictCalendar),
         "strict_calendar should treat 0200 as earlier than 2207.");
}

auto TestNormalDaySequence(TestState& state) -> void {
  using tracer::core::application::pipeline::record_time_order::IsStrictlyAfter;

  constexpr std::array<std::string_view, 5> kTimes = {
      "0656", "0904", "1307", "1920", "2207"};
  for (std::size_t i = 1; i < kTimes.size(); ++i) {
    Expect(state,
           IsStrictlyAfter(kTimes[i], kTimes[i - 1],
                           TimeOrderMode::kStrictCalendar),
           "strict_calendar should keep daytime sequence strictly increasing.");
    Expect(state,
           IsStrictlyAfter(kTimes[i], kTimes[i - 1],
                           TimeOrderMode::kLogicalDay0600),
           "logical_day_0600 should keep daytime sequence strictly increasing.");
  }
}

auto TestLogicalDayBoundaryAndEquality(TestState& state) -> void {
  using tracer::core::application::pipeline::record_time_order::IsStrictlyAfter;

  Expect(state,
         IsStrictlyAfter("0559", "2359", TimeOrderMode::kLogicalDay0600),
         "logical_day_0600 should roll times before 0600 into the next logical day.");
  Expect(state,
         !IsStrictlyAfter("0600", "2359", TimeOrderMode::kLogicalDay0600),
         "logical_day_0600 should keep 0600 anchored to the same logical day boundary.");
  Expect(state,
         !IsStrictlyAfter("0600", "2359", TimeOrderMode::kStrictCalendar),
         "strict_calendar should keep 0600 earlier than 2359.");

  Expect(state,
         !IsStrictlyAfter("0904", "0904", TimeOrderMode::kStrictCalendar),
         "strict_calendar should reject equal timestamps as strictly increasing.");
  Expect(state,
         !IsStrictlyAfter("0904", "0904", TimeOrderMode::kLogicalDay0600),
         "logical_day_0600 should reject equal timestamps as strictly increasing.");
}

}  // namespace

auto RunRecordTimeOrderModeTests(TestState& state) -> void {
  TestLogicalDayModeCrossMidnight(state);
  TestNormalDaySequence(state);
  TestLogicalDayBoundaryAndEquality(state);
}

}  // namespace tracer_core::application::tests
