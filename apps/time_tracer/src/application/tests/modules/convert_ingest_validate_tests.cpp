// application/tests/modules/convert_ingest_validate_tests.cpp
#include "application/tests/modules/test_modules.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"
#include "domain/logic/converter/convert/core/converter_core.hpp"

namespace time_tracer::application::tests {

using time_tracer::core::dto::ConvertRequest;
using time_tracer::core::dto::IngestRequest;

namespace {

auto TestConvertResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto core_api = BuildCoreApiForTest(workflow_handler, report_handler);

  const ConvertRequest kRequest = {.input_path = "source-path",
                                   .date_check_mode = DateCheckMode::kFull,
                                   .save_processed_output = true,
                                   .validate_logic = false,
                                   .validate_structure = true};

  const auto kSuccess = core_api.RunConvert(kRequest);
  Expect(state, kSuccess.ok, "RunConvert should return ok on success.");
  Expect(state, kSuccess.error_message.empty(),
         "RunConvert success should have empty error_message.");
  Expect(state, workflow_handler.convert_call_count == 1,
         "RunConvert should call workflow handler once.");
  Expect(state, workflow_handler.last_converter_input == kRequest.input_path,
         "RunConvert should forward input path.");
  Expect(state,
         workflow_handler.last_converter_options.date_check_mode ==
             kRequest.date_check_mode,
         "RunConvert should forward date_check_mode.");
  Expect(state,
         workflow_handler.last_converter_options.save_processed_output ==
             kRequest.save_processed_output,
         "RunConvert should forward save_processed_output.");
  Expect(state,
         workflow_handler.last_converter_options.validate_logic ==
             kRequest.validate_logic,
         "RunConvert should forward validate_logic.");
  Expect(state,
         workflow_handler.last_converter_options.validate_structure ==
             kRequest.validate_structure,
         "RunConvert should forward validate_structure.");

  workflow_handler.fail_convert = true;
  const auto kFailure = core_api.RunConvert(kRequest);
  Expect(state, !kFailure.ok,
         "RunConvert should return failed DTO on exception.");
  Expect(state, Contains(kFailure.error_message, "RunConvert failed"),
         "RunConvert failure should include operation name.");
  Expect(state, Contains(kFailure.error_message, "convert failed"),
         "RunConvert failure should include dependency error details.");
}

auto TestIngestResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto core_api = BuildCoreApiForTest(workflow_handler, report_handler);

  const IngestRequest kRequest = {.input_path = "source-folder",
                                  .date_check_mode = DateCheckMode::kContinuity,
                                  .save_processed_output = true,
                                  .ingest_mode = IngestMode::kStandard};

  const auto kSuccess = core_api.RunIngest(kRequest);
  Expect(state, kSuccess.ok, "RunIngest should return ok on success.");
  Expect(state, kSuccess.error_message.empty(),
         "RunIngest success should have empty error_message.");
  Expect(state, workflow_handler.ingest_call_count == 1,
         "RunIngest should call workflow handler once.");
  Expect(state, workflow_handler.last_ingest_input == kRequest.input_path,
         "RunIngest should forward input path.");
  Expect(state, workflow_handler.last_ingest_mode == kRequest.date_check_mode,
         "RunIngest should forward date_check_mode.");
  Expect(state,
         workflow_handler.last_ingest_save_processed ==
             kRequest.save_processed_output,
         "RunIngest should forward save_processed_output.");
  Expect(state,
         workflow_handler.last_ingest_import_mode == kRequest.ingest_mode,
         "RunIngest should forward ingest_mode.");

  workflow_handler.fail_ingest = true;
  const auto kFailure = core_api.RunIngest(kRequest);
  Expect(state, !kFailure.ok,
         "RunIngest should return failed DTO on exception.");
  Expect(state, Contains(kFailure.error_message, "RunIngest failed"),
         "RunIngest failure should include operation name.");
  Expect(state, Contains(kFailure.error_message, "ingest failed"),
         "RunIngest failure should include dependency error details.");
}

auto TestValidateResponses(TestState& state) -> void {
  FakeWorkflowHandler workflow_handler;
  FakeReportHandler report_handler;
  auto core_api = BuildCoreApiForTest(workflow_handler, report_handler);

  const auto kStructureOk =
      core_api.RunValidateStructure({.input_path = "input-folder"});
  Expect(state, kStructureOk.ok,
         "RunValidateStructure should return ok on success.");
  Expect(state, workflow_handler.validate_structure_call_count == 1,
         "RunValidateStructure should call workflow handler once.");
  Expect(state,
         workflow_handler.last_validate_structure_input == "input-folder",
         "RunValidateStructure should forward input path.");

  workflow_handler.fail_validate_structure = true;
  const auto kStructureFailure =
      core_api.RunValidateStructure({.input_path = "bad-input"});
  Expect(state, !kStructureFailure.ok,
         "RunValidateStructure should return failed DTO when handler throws.");
  Expect(state,
         Contains(kStructureFailure.error_message, "RunValidateStructure"),
         "RunValidateStructure failure should include operation name.");

  const auto kLogicOk = core_api.RunValidateLogic(
      {.input_path = "logic-folder", .date_check_mode = DateCheckMode::kFull});
  Expect(state, kLogicOk.ok, "RunValidateLogic should return ok on success.");
  Expect(state, workflow_handler.validate_logic_call_count == 1,
         "RunValidateLogic should call workflow handler once.");
  Expect(state, workflow_handler.last_validate_logic_input == "logic-folder",
         "RunValidateLogic should forward input path.");
  Expect(state,
         workflow_handler.last_validate_logic_mode == DateCheckMode::kFull,
         "RunValidateLogic should forward date_check_mode.");

  workflow_handler.fail_validate_logic = true;
  const auto kLogicFailure = core_api.RunValidateLogic(
      {.input_path = "bad-logic", .date_check_mode = DateCheckMode::kNone});
  Expect(state, !kLogicFailure.ok,
         "RunValidateLogic should return failed DTO when handler throws.");
  Expect(state, Contains(kLogicFailure.error_message, "RunValidateLogic"),
         "RunValidateLogic failure should include operation name.");
}

auto TestContinuationDayPreservesFirstSegment(TestState& state) -> void {
  ConverterConfig config;
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-01";
  previous_day.rawEvents.push_back(RawEvent{.endTimeStr = "23:30"});

  DailyLog continuation_day;
  continuation_day.date = "2026-02-02";
  continuation_day.isContinuation = true;
  continuation_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "07:00", .description = "study_cpp"});

  processor.Process(previous_day, continuation_day);

  Expect(state, continuation_day.processedActivities.size() == 1,
         "Continuation day should keep first segment after conversion.");
  if (continuation_day.processedActivities.empty()) {
    return;
  }

  const auto& first_activity = continuation_day.processedActivities.front();
  Expect(state, first_activity.start_time_str == "23:30",
         "Continuation day first segment should start from previous day end.");
  Expect(state, first_activity.end_time_str == "07:00",
         "Continuation day first segment should end at first raw event time.");
  Expect(state, first_activity.duration_seconds > 0,
         "Continuation day first segment duration should be positive.");
}

}  // namespace

auto RunConvertIngestValidateTests(TestState& state) -> void {
  TestConvertResponses(state);
  TestIngestResponses(state);
  TestValidateResponses(state);
  TestContinuationDayPreservesFirstSegment(state);
}

}  // namespace time_tracer::application::tests
