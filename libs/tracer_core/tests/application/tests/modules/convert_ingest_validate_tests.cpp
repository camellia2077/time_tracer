import tracer.core.domain.logic.converter.core;
import tracer.core.domain.logic.converter.log_processor;
import tracer.core.domain.types.converter_config;

// application/tests/modules/convert_ingest_validate_tests.cpp
#include "application/tests/modules/pipeline_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

using tracer::core::domain::model::ActivityRecordKind;
using tracer::core::domain::modlogic::converter::DayProcessor;
using tracer::core::domain::modlogic::converter::LogProcessor;
using tracer::core::domain::modtypes::ConverterConfig;

using tracer_core::core::dto::ConvertRequest;
using tracer_core::core::dto::IngestRequest;

namespace {

using RawEventKindType = decltype(RawEvent{}.kind);

auto BuildTimelineTestConfig() -> ConverterConfig {
  ConverterConfig config;
  config.text_mapping["breakfast"] = "breakfast";
  config.text_mapping["english"] = "english";
  config.text_mapping["game"] = "game";
  config.text_mapping["internet"] = "internet";
  config.text_mapping["lunch"] = "lunch";
  config.text_mapping["math"] = "math";
  config.text_mapping["sleep"] = "sleep";
  config.text_mapping["study"] = "study";
  config.text_mapping["wake"] = "wake";
  config.sleep_inference.wake_keywords = {"wake"};
  return config;
}

auto TestConvertResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler);

  const ConvertRequest kRequest = {.input_path = "source-path",
                                   .date_check_mode = DateCheckMode::kFull,
                                   .save_processed_output = true,
                                   .validate_logic = false,
                                   .validate_structure = true};

  const auto kSuccess = runtime_api.pipeline().RunConvert(kRequest);
  Expect(state, kSuccess.ok, "RunConvert should return ok on success.");
  Expect(state, kSuccess.error_message.empty(),
         "RunConvert success should have empty error_message.");
  Expect(state, pipeline_workflow.convert_call_count == 1,
         "RunConvert should call workflow handler once.");
  Expect(state, pipeline_workflow.last_converter_input == kRequest.input_path,
         "RunConvert should forward input path.");
  Expect(state,
         pipeline_workflow.last_converter_options.date_check_mode ==
             kRequest.date_check_mode,
         "RunConvert should forward date_check_mode.");
  Expect(state,
         pipeline_workflow.last_converter_options.save_processed_output ==
             kRequest.save_processed_output,
         "RunConvert should forward save_processed_output.");
  Expect(state,
         pipeline_workflow.last_converter_options.validate_logic ==
             kRequest.validate_logic,
         "RunConvert should forward validate_logic.");
  Expect(state,
         pipeline_workflow.last_converter_options.validate_structure ==
             kRequest.validate_structure,
         "RunConvert should forward validate_structure.");
  Expect(state,
         !pipeline_workflow.last_converter_options
              .run_structure_validation_before_conversion,
         "RunConvert should not enable structure precheck when validate_logic "
         "is false.");

  const ConvertRequest kLogicRequest = {.input_path = "source-path",
                                        .date_check_mode = DateCheckMode::kNone,
                                        .save_processed_output = false,
                                        .validate_logic = true,
                                        .validate_structure = false};
  const auto kLogicSuccess = runtime_api.pipeline().RunConvert(kLogicRequest);
  Expect(state, kLogicSuccess.ok,
         "RunConvert should succeed for validate_logic precheck request.");
  Expect(state,
         pipeline_workflow.last_converter_options
             .run_structure_validation_before_conversion,
         "RunConvert should enable structure precheck when validate_logic is "
         "true.");

  pipeline_workflow.fail_convert = true;
  const auto kFailure = runtime_api.pipeline().RunConvert(kRequest);
  Expect(state, !kFailure.ok,
         "RunConvert should return failed DTO on exception.");
  Expect(state, Contains(kFailure.error_message, "RunConvert failed"),
         "RunConvert failure should include operation name.");
  Expect(state, Contains(kFailure.error_message, "convert failed"),
         "RunConvert failure should include dependency error details.");
}

auto TestIngestResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler);

  const IngestRequest kRequest = {.input_path = "source-folder",
                                  .date_check_mode = DateCheckMode::kContinuity,
                                  .save_processed_output = true,
                                  .ingest_mode = IngestMode::kStandard};

  const auto kSuccess = runtime_api.pipeline().RunIngest(kRequest);
  Expect(state, kSuccess.ok, "RunIngest should return ok on success.");
  Expect(state, kSuccess.error_message.empty(),
         "RunIngest success should have empty error_message.");
  Expect(state, pipeline_workflow.ingest_call_count == 1,
         "RunIngest should call workflow handler once.");
  Expect(state, pipeline_workflow.last_ingest_input == kRequest.input_path,
         "RunIngest should forward input path.");
  Expect(state, pipeline_workflow.last_ingest_mode == kRequest.date_check_mode,
         "RunIngest should forward date_check_mode.");
  Expect(state,
         pipeline_workflow.last_ingest_save_processed ==
             kRequest.save_processed_output,
         "RunIngest should forward save_processed_output.");
  Expect(state,
         pipeline_workflow.last_ingest_import_mode == kRequest.ingest_mode,
         "RunIngest should forward ingest_mode.");

  pipeline_workflow.fail_ingest = true;
  const auto kFailure = runtime_api.pipeline().RunIngest(kRequest);
  Expect(state, !kFailure.ok,
         "RunIngest should return failed DTO on exception.");
  Expect(state, Contains(kFailure.error_message, "RunIngest failed"),
         "RunIngest failure should include operation name.");
  Expect(state, Contains(kFailure.error_message, "ingest failed"),
         "RunIngest failure should include dependency error details.");
}

auto TestValidateResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeInsightsHandler insights_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, insights_handler);

  const auto kStructureOk = runtime_api.pipeline().RunValidateStructure(
      {.input_path = "input-folder"});
  Expect(state, kStructureOk.ok,
         "RunValidateStructure should return ok on success.");
  Expect(state, pipeline_workflow.validate_structure_call_count == 1,
         "RunValidateStructure should call workflow handler once.");
  Expect(state,
         pipeline_workflow.last_validate_structure_input == "input-folder",
         "RunValidateStructure should forward input path.");

  pipeline_workflow.fail_validate_structure = true;
  const auto kStructureFailure =
      runtime_api.pipeline().RunValidateStructure({.input_path = "bad-input"});
  Expect(state, !kStructureFailure.ok,
         "RunValidateStructure should return failed DTO when handler throws.");
  Expect(state,
         Contains(kStructureFailure.error_message, "RunValidateStructure"),
         "RunValidateStructure failure should include operation name.");

  const auto kLogicOk = runtime_api.pipeline().RunValidateLogic(
      {.input_path = "logic-folder", .date_check_mode = DateCheckMode::kFull});
  Expect(state, kLogicOk.ok, "RunValidateLogic should return ok on success.");
  Expect(state, pipeline_workflow.validate_logic_call_count == 1,
         "RunValidateLogic should call workflow handler once.");
  Expect(state, pipeline_workflow.last_validate_logic_input == "logic-folder",
         "RunValidateLogic should forward input path.");
  Expect(state,
         pipeline_workflow.last_validate_logic_mode == DateCheckMode::kFull,
         "RunValidateLogic should forward date_check_mode.");

  pipeline_workflow.fail_validate_logic = true;
  const auto kLogicFailure = runtime_api.pipeline().RunValidateLogic(
      {.input_path = "bad-logic", .date_check_mode = DateCheckMode::kNone});
  Expect(state, !kLogicFailure.ok,
         "RunValidateLogic should return failed DTO when handler throws.");
  Expect(state, Contains(kLogicFailure.error_message, "RunValidateLogic"),
         "RunValidateLogic failure should include operation name.");

  const auto kAtomicRecordOk =
      runtime_api.pipeline().RunRecordActivityAtomically(
          {.target_date_iso = "2026-03-29",
           .raw_activity_name = "study",
           .remark = "remark",
           .preferred_txt_path = "2026/2026-03.txt",
           .date_check_mode = DateCheckMode::kNone,
           .time_order_mode = TimeOrderMode::kLogicalDay0600});
  Expect(state, kAtomicRecordOk.ok,
         "RunRecordActivityAtomically should return ok on success.");
  Expect(state, pipeline_workflow.record_activity_atomically_call_count == 1,
         "RunRecordActivityAtomically should call workflow handler once.");
  Expect(state,
         pipeline_workflow.last_record_activity_request.target_date_iso ==
             "2026-03-29",
         "RunRecordActivityAtomically should forward target_date_iso.");
  Expect(state,
         pipeline_workflow.last_record_activity_request.raw_activity_name ==
             "study",
         "RunRecordActivityAtomically should forward raw_activity_name.");
  Expect(state,
         pipeline_workflow.last_record_activity_request.time_order_mode ==
             TimeOrderMode::kLogicalDay0600,
         "RunRecordActivityAtomically should forward time_order_mode.");

  const auto kAtomicRecordDefaultMode =
      runtime_api.pipeline().RunRecordActivityAtomically(
          {.target_date_iso = "2026-03-29",
           .raw_activity_name = "study",
           .remark = "",
           .preferred_txt_path = "",
           .date_check_mode = DateCheckMode::kNone});
  Expect(state, kAtomicRecordDefaultMode.ok,
         "RunRecordActivityAtomically should succeed when time_order_mode is "
         "omitted.");
  Expect(state,
         pipeline_workflow.last_record_activity_request.time_order_mode ==
             TimeOrderMode::kStrictCalendar,
         "RunRecordActivityAtomically should default time_order_mode to "
         "strict_calendar.");

  pipeline_workflow.fail_record_activity_atomically = true;
  const auto kAtomicRecordFailure =
      runtime_api.pipeline().RunRecordActivityAtomically(
          {.target_date_iso = "2026-03-29",
           .raw_activity_name = "study",
           .remark = "",
           .preferred_txt_path = "",
           .date_check_mode = DateCheckMode::kContinuity,
           .time_order_mode = TimeOrderMode::kStrictCalendar});
  Expect(state, !kAtomicRecordFailure.ok,
         "RunRecordActivityAtomically should return failed DTO when handler "
         "throws.");
  Expect(state,
         Contains(kAtomicRecordFailure.message,
                  "RunRecordActivityAtomically failed"),
         "RunRecordActivityAtomically failure should include operation name.");

  const auto kDayRemarkOk = runtime_api.pipeline().RunUpdateDayRemarkAtomically(
      {.target_date_iso = "2026-03-29",
       .remark = "day remark first\nday remark second",
       .preferred_txt_path = "2026/2026-03.txt",
       .date_check_mode = DateCheckMode::kNone});
  Expect(state, kDayRemarkOk.ok,
         "RunUpdateDayRemarkAtomically should return ok on success.");
  Expect(state, pipeline_workflow.update_day_remark_atomically_call_count == 1,
         "RunUpdateDayRemarkAtomically should call workflow handler once.");
  Expect(state,
         pipeline_workflow.last_update_day_remark_request.target_date_iso ==
             "2026-03-29",
         "RunUpdateDayRemarkAtomically should forward target_date_iso.");
  Expect(state,
         pipeline_workflow.last_update_day_remark_request.remark ==
             "day remark first\nday remark second",
         "RunUpdateDayRemarkAtomically should forward multiline remark.");
}

auto TestContinuationDayPreservesFirstSegment(TestState& state) -> void {
  ConverterConfig config;
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-01";
  previous_day.rawEvents.push_back(RawEvent{.endTimeStr = "23:30:00"});

  DailyLog continuation_day;
  continuation_day.date = "2026-02-02";
  continuation_day.isContinuation = true;
  continuation_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "07:00:00", .description = "study_cpp"});

  processor.Process(previous_day, continuation_day);

  Expect(state, continuation_day.processedActivities.size() == 1,
         "Continuation day should keep first segment after conversion.");
  if (continuation_day.processedActivities.empty()) {
    return;
  }

  const auto& first_activity = continuation_day.processedActivities.front();
  Expect(state, first_activity.start_time_str == "23:30:00",
         "Continuation day first segment should start from previous day end.");
  Expect(state, first_activity.end_time_str == "07:00:00",
         "Continuation day first segment should end at first raw event time.");
  Expect(state, first_activity.duration_seconds > 0,
         "Continuation day first segment duration should be positive.");
}

auto TestIntervalDayBuildsExplicitRecords(TestState& state) -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  config.text_mapping["sleep"] = "sleep";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-02";

  DailyLog interval_day;
  interval_day.date = "2026-02-03";
  interval_day.isContinuation = true;
  interval_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Interval,
                                            .startTimeStr = std::string("09:00:00"),
                                            .endTimeStr = "10:30:00",
                                            .description = "study"});
  interval_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Interval,
                                            .startTimeStr = std::string("14:01:00"),
                                            .endTimeStr = "19:00:00",
                                            .description = "sleep"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 2,
         "Pure interval day should produce one record per explicit interval.");
  if (interval_day.processedActivities.size() != 2) {
    return;
  }

  const auto& first = interval_day.processedActivities[0];
  const auto& second = interval_day.processedActivities[1];
  Expect(state,
         first.start_time_str == "09:00:00" && first.end_time_str == "10:30:00",
         "First explicit interval should keep its authored start/end.");
  Expect(
      state,
      second.start_time_str == "14:01:00" && second.end_time_str == "19:00:00",
      "Second explicit interval should keep its authored start/end.");
  Expect(state, first.duration_seconds == 90 * 60,
         "First explicit interval duration should be 90 minutes.");
  Expect(state, second.duration_seconds == ((4 * 60) + 59) * 60,
         "Second explicit interval duration should be 4h59m.");
}

auto TestMixedDayUsesIntervalEndAsNextPointBoundary(TestState& state) -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  config.text_mapping["sleep"] = "sleep";
  config.text_mapping["wake"] = "wake";
  config.sleep_inference.wake_keywords = {"wake"};
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-03";

  DailyLog mixed_day;
  mixed_day.date = "2026-02-04";
  mixed_day.getupTime = "06:06:00";
  mixed_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Point,
                                         .endTimeStr = "06:06:00",
                                         .description = "wake"});
  mixed_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Interval,
                                         .startTimeStr = std::string("09:00:00"),
                                         .endTimeStr = "10:30:00",
                                         .description = "study"});
  mixed_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Point,
                                         .endTimeStr = "13:53:00",
                                         .description = "sleep"});

  processor.Process(previous_day, mixed_day);

  Expect(state, mixed_day.processedActivities.size() == 2,
         "Mixed day should emit one explicit interval and one derived point "
         "segment.");
  if (mixed_day.processedActivities.size() != 2) {
    return;
  }

  const auto& trailing_sleep = mixed_day.processedActivities.back();
  Expect(state, trailing_sleep.start_time_str == "10:30:00",
         "Point event after interval should start from the interval end "
         "boundary.");
  Expect(state, trailing_sleep.end_time_str == "13:53:00",
         "Point event after interval should end at the point authored time.");
}

auto TestContinuationDayAllowsGapBeforeFirstInterval(TestState& state) -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-04";
  previous_day.rawEvents.push_back(RawEvent{.endTimeStr = "23:30:00"});

  DailyLog continuation_day;
  continuation_day.date = "2026-02-05";
  continuation_day.isContinuation = true;
  continuation_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("09:00:00"),
               .endTimeStr = "10:30:00",
               .description = "study"});

  processor.Process(previous_day, continuation_day);

  Expect(state, continuation_day.processedActivities.size() == 1,
         "Continuation day with first interval should keep the explicit "
         "interval.");
  if (continuation_day.processedActivities.empty()) {
    return;
  }

  const auto& first_activity = continuation_day.processedActivities.front();
  Expect(state,
         first_activity.start_time_str == "09:00:00" &&
             first_activity.end_time_str == "10:30:00",
         "Continuation interval should not be rewritten to the previous-day "
         "boundary.");
}

auto TestWrappedIntervalPreservesCrossMidnightDuration(TestState& state)
    -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-05";

  DailyLog interval_day;
  interval_day.date = "2026-02-06";
  interval_day.isContinuation = true;
  interval_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Interval,
                                            .startTimeStr = std::string("21:32:00"),
                                            .endTimeStr = "01:35:00",
                                            .description = "study"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 1,
         "Wrapped interval should still produce one explicit record.");
  if (interval_day.processedActivities.empty()) {
    return;
  }

  const auto& activity = interval_day.processedActivities.front();
  Expect(state, activity.start_time_str == "21:32:00",
         "Wrapped interval should preserve the authored start boundary.");
  Expect(state, activity.end_time_str == "01:35:00",
         "Wrapped interval should preserve the authored end boundary.");
  Expect(state, activity.duration_seconds == ((4 * 60) + 3) * 60,
         "Wrapped interval duration should span across midnight.");
}

auto TestCrossMidnightIntervalDoesNotUseWrapThreshold(TestState& state)
    -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-06";

  DailyLog interval_day;
  interval_day.date = "2026-02-07";
  interval_day.isContinuation = true;
  interval_day.rawEvents.push_back(RawEvent{.kind = RawEventKindType::Interval,
                                            .startTimeStr = std::string("10:30:00"),
                                            .endTimeStr = "09:00:00",
                                            .description = "study",
                                            .remark = "@allow-long"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 1,
         "Cross-midnight interval should produce one explicit record.");
  if (interval_day.processedActivities.empty()) {
    return;
  }

  const auto& activity = interval_day.processedActivities.front();
  Expect(
      state, activity.start_time_str == "10:30:00",
      "Cross-midnight interval should preserve the authored start boundary.");
  Expect(state, activity.end_time_str == "09:00:00",
         "Cross-midnight interval should preserve the authored end boundary.");
  Expect(state, activity.duration_seconds == ((22 * 60) + 30) * 60,
         "Cross-midnight interval should be interpreted as ending next day.");
  Expect(state,
         activity.remark.has_value() &&
             activity.remark.value().find("@allow-long") != std::string::npos,
         "Cross-midnight interval should preserve @allow-long remark.");
}

auto TestPureIntervalTxtParsesSparseDaysWithoutContext(TestState& state)
    -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent("interval-month.txt",
                                                     "y2026\n"
                                                     "m03\n"
                                                     "d0301\n"
                                                     "0901-1200math\n"
                                                     "1220-1409lunch\n"
                                                     "1608-1900english\n"
                                                     "\n"
                                                     "d0305\n"
                                                     "0803-0907game\n"
                                                     "1320-1409lunch\n"
                                                     "1608-1900english\n");

  Expect(state, result.success, "Pure interval TXT conversion should succeed.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state, month_it != result.processed_data.end(),
         "Pure interval TXT conversion should keep the March bucket.");
  if (month_it == result.processed_data.end()) {
    return;
  }

  const auto& days = month_it->second;
  Expect(state, days.size() == 2,
         "Pure interval TXT should preserve sparse authored days.");
  if (days.size() != 2) {
    return;
  }

  const auto& march_first = days[0];
  const auto& march_fifth = days[1];
  Expect(state, march_first.processedActivities.size() == 3,
         "March 1 pure interval day should emit three records.");
  Expect(state, march_fifth.processedActivities.size() == 3,
         "March 5 pure interval day should emit three records.");
  Expect(state, !march_fifth.isContinuation,
         "First interval event should not mark the day as overnight "
         "continuation.");
  if (march_fifth.processedActivities.empty()) {
    return;
  }

  const auto& first_record = march_fifth.processedActivities.front();
  Expect(state,
         first_record.start_time_str == "08:03:00" &&
             first_record.end_time_str == "09:07:00",
         "Sparse pure interval day should use its own explicit first range.");
}

auto TestMixedTxtUsesIntervalEndAndLeavesGapsUnrecorded(TestState& state)
    -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent("mixed-month.txt",
                                                     "y2026\n"
                                                     "m03\n"
                                                     "d0301\n"
                                                     "0809breakfast\n"
                                                     "1200game\n"
                                                     "1230-1304sleep\n"
                                                     "1404-1623study\n"
                                                     "1809internet\n");

  Expect(state, result.success, "Mixed TXT conversion should succeed.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state, month_it != result.processed_data.end(),
         "Mixed TXT conversion should keep the March bucket.");
  if (month_it == result.processed_data.end() || month_it->second.empty()) {
    return;
  }

  const auto& day = month_it->second.front();
  Expect(state, day.processedActivities.size() == 5,
         "Mixed day should retain the context-free first point and leave gaps "
         "empty.");
  if (day.processedActivities.size() != 5) {
    return;
  }

  const auto& breakfast = day.processedActivities[0];
  const auto& game = day.processedActivities[1];
  const auto& sleep = day.processedActivities[2];
  const auto& study = day.processedActivities[3];
  const auto& internet = day.processedActivities[4];
  Expect(state,
         breakfast.kind == ActivityRecordKind::kEndOnly &&
             breakfast.start_time_str.empty() &&
             breakfast.end_time_str == "08:09:00",
         "Context-free first point should be retained as end-only.");
  Expect(state,
         game.start_time_str == "08:09:00" && game.end_time_str == "12:00:00" &&
             game.project_path == "game",
         "Point event should use the previous point boundary.");
  Expect(state,
         sleep.start_time_str == "12:30:00" && sleep.end_time_str == "13:04:00",
         "Interval event should keep its explicit range after a point gap.");
  Expect(state,
         study.start_time_str == "14:04:00" && study.end_time_str == "16:23:00",
         "Later interval event should keep its explicit range after a gap.");
  Expect(state,
         internet.start_time_str == "16:23:00" &&
             internet.end_time_str == "18:09:00",
         "Point event after interval should start at the interval end.");
}

auto TestMixedTxtContiguousSampleUsesSharedTimeline(TestState& state) -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result =
      processor.ProcessSourceContent("mixed-contiguous-month.txt",
                                     "y2026\n"
                                     "m03\n"
                                     "d0301\n"
                                     "0809breakfast\n"
                                     "1200game\n"
                                     "1200-1304sleep\n"
                                     "1304-1623study\n"
                                     "1809internet\n");

  Expect(state, result.success,
         "Contiguous mixed TXT conversion should succeed.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state, month_it != result.processed_data.end(),
         "Contiguous mixed TXT conversion should keep the March bucket.");
  if (month_it == result.processed_data.end() || month_it->second.empty()) {
    return;
  }

  const auto& day = month_it->second.front();
  Expect(state, day.processedActivities.size() == 5,
         "Contiguous mixed day should retain the first end-only activity.");
  if (day.processedActivities.size() != 5) {
    return;
  }

  const auto& breakfast = day.processedActivities[0];
  const auto& game = day.processedActivities[1];
  const auto& sleep = day.processedActivities[2];
  const auto& study = day.processedActivities[3];
  const auto& internet = day.processedActivities[4];
  Expect(state,
         breakfast.kind == ActivityRecordKind::kEndOnly &&
             breakfast.start_time_str.empty() &&
             breakfast.end_time_str == "08:09:00",
         "Contiguous sample should retain the context-free first point.");
  Expect(state,
         game.project_path == "game" && game.start_time_str == "08:09:00" &&
             game.end_time_str == "12:00:00",
         "First materialized point activity should use the previous point "
         "boundary.");
  Expect(state,
         sleep.project_path == "sleep" && sleep.start_time_str == "12:00:00" &&
             sleep.end_time_str == "13:04:00",
         "Contiguous interval should keep its explicit start and end.");
  Expect(
      state,
      study.project_path == "study" && study.start_time_str == "13:04:00" &&
          study.end_time_str == "16:23:00",
      "Following contiguous interval should keep its explicit start and end.");
  Expect(state,
         internet.project_path == "internet" &&
             internet.start_time_str == "16:23:00" &&
             internet.end_time_str == "18:09:00",
         "Point event after contiguous intervals should use the last interval "
         "end.");
}

auto TestMixedTxtPointAfterCrossMidnightIntervalUsesExpandedBoundary(
    TestState& state) -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result =
      processor.ProcessSourceContent("mixed-cross-midnight-month.txt",
                                     "y2026\n"
                                     "m03\n"
                                     "d0302\n"
                                     "2132-0135study\n"
                                     "2350game\n");

  Expect(state, result.success,
         "Cross-midnight mixed TXT conversion should succeed.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state, month_it != result.processed_data.end(),
         "Cross-midnight mixed TXT conversion should keep the March bucket.");
  if (month_it == result.processed_data.end() || month_it->second.empty()) {
    return;
  }

  const auto& day = month_it->second.front();
  Expect(state, day.processedActivities.size() == 2,
         "Cross-midnight mixed day should emit interval and trailing point "
         "activity.");
  if (day.processedActivities.size() != 2) {
    return;
  }

  const auto& study = day.processedActivities[0];
  const auto& game = day.processedActivities[1];
  Expect(state,
         study.project_path == "study" && study.start_time_str == "21:32:00" &&
             study.end_time_str == "01:35:00" &&
             study.duration_seconds == ((4 * 60) + 3) * 60,
         "Cross-midnight interval should keep next-day duration.");
  Expect(state,
         game.project_path == "game" && game.start_time_str == "01:35:00" &&
             game.end_time_str == "23:50:00" &&
             game.duration_seconds == ((22 * 60) + 15) * 60,
         "Point after cross-midnight interval should use the expanded previous "
         "end boundary.");
}

auto TestPointWithoutContextProducesEndOnly(TestState& state) -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  config.text_mapping["meal"] = "meal";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-01";

  DailyLog day;
  day.date = "2026-02-02";
  day.isContinuation = true;
  day.rawEvents.push_back(
      RawEvent{.endTimeStr = "09:00:00", .description = "study"});
  day.rawEvents.push_back(
      RawEvent{.endTimeStr = "10:00:00", .description = "meal"});

  processor.Process(previous_day, day);

  Expect(state, day.processedActivities.size() == 2,
         "A point event without context should remain in the activity facts.");
  if (day.processedActivities.size() != 2) {
    return;
  }

  const auto& end_only = day.processedActivities[0];
  const auto& following = day.processedActivities[1];
  Expect(state, end_only.kind == ActivityRecordKind::kEndOnly,
         "The first point without context should be marked end-only.");
  Expect(state,
         end_only.start_time_str.empty() &&
             end_only.end_time_str == "09:00:00" &&
             end_only.duration_seconds == 0,
         "End-only should keep only its end boundary and no duration.");
  Expect(state,
         following.kind == ActivityRecordKind::kInterval &&
             following.start_time_str == "09:00:00" &&
             following.end_time_str == "10:00:00" &&
             following.duration_seconds == 60 * 60,
         "A following point should use end-only's end as its start boundary.");
}

auto TestEndOnlyBoundaryCanBeInheritedAcrossAdjacentDay(TestState& state)
    -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  config.text_mapping["meal"] = "meal";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-01";
  previous_day.isContinuation = true;
  previous_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "18:00:00", .description = "study"});
  DailyLog no_context_day;
  processor.Process(no_context_day, previous_day);

  Expect(state,
         previous_day.processedActivities.size() == 1 &&
             previous_day.processedActivities.front().kind ==
                 ActivityRecordKind::kEndOnly,
         "The previous day should materialize its context-free point as "
         "end-only.");

  DailyLog current_day;
  current_day.date = "2026-02-02";
  current_day.isContinuation = true;
  current_day.rawEvents.push_back(
      RawEvent{.endTimeStr = "09:00:00", .description = "meal"});
  processor.Process(previous_day, current_day);

  Expect(state, current_day.processedActivities.size() == 1,
         "The adjacent day should keep its first point activity.");
  if (current_day.processedActivities.empty()) {
    return;
  }
  const auto& activity = current_day.processedActivities.front();
  Expect(state,
         activity.kind == ActivityRecordKind::kInterval &&
             activity.start_time_str == "18:00:00" &&
             activity.end_time_str == "09:00:00" &&
             activity.duration_seconds == 15 * 60 * 60,
         "The next day should inherit the previous end-only boundary.");
}

auto TestMultilineRemarksUsePhysicalContinuationLines(TestState& state)
    -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result =
      processor.ProcessSourceContent("multiline-remarks-month.txt",
                                     "y2026\n"
                                     "m03\n"
                                     "d0301\n"
                                     "// day remark first\n"
                                     "// day remark second\n"
                                     "0600wake\n"
                                     "0800study // activity remark first\n"
                                     "// activity remark second\n"
                                     "\n"
                                     "1000sleep // literal \\n text\n");

  Expect(state, result.success,
         "Physical // continuation lines should parse successfully.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state, month_it != result.processed_data.end(),
         "Multiline remark TXT should produce the March bucket.");
  if (month_it == result.processed_data.end() || month_it->second.empty()) {
    return;
  }

  const auto& day = month_it->second.front();
  Expect(state,
         day.generalRemarks.size() == 2 &&
             day.generalRemarks[0] == "day remark first" &&
             day.generalRemarks[1] == "day remark second",
         "Day remark lines should remain ordered physical lines before memory "
         "merge.");
  Expect(state, day.processedActivities.size() == 2,
         "Multiline remark sample should produce two non-wake activities.");
  if (day.processedActivities.size() != 2) {
    return;
  }

  const auto& study = day.processedActivities.front();
  const auto& sleep = day.processedActivities.back();
  Expect(state,
         study.remark.has_value() &&
             study.remark.value() ==
                 "activity remark first\nactivity remark second",
         "Activity continuation lines should join with a real LF.");
  Expect(state,
         sleep.remark.has_value() && sleep.remark.value() == "literal \\n text",
         "Literal backslash-n text should not be decoded.");
}

auto TestHashAndSemicolonAreNotRemarkDelimiters(TestState& state) -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);
  const auto result =
      processor.ProcessSourceContent("removed-remark-delimiters.txt",
                                     "y2026\n"
                                     "m03\n"
                                     "d0301\n"
                                     "0600wake\n"
                                     "0800study # old syntax\n"
                                     "1000sleep ; old syntax\n");

  Expect(state, result.success,
         "Hash and semicolon should remain valid activity text.");
  const auto month_it = result.processed_data.find("2026-03");
  Expect(state,
         month_it != result.processed_data.end() && !month_it->second.empty(),
         "Literal hash and semicolon sample should produce a day.");
  if (month_it == result.processed_data.end() || month_it->second.empty()) {
    return;
  }
  const auto& activities = month_it->second.front().processedActivities;
  Expect(state, activities.size() == 2,
         "Literal hash and semicolon sample should produce two activities.");
  if (activities.size() != 2) {
    return;
  }
  Expect(state,
         activities[0].project_path == "study # old syntax" &&
             !activities[0].remark.has_value() &&
             activities[1].project_path == "sleep ; old syntax" &&
             !activities[1].remark.has_value(),
         "Hash and semicolon must remain in activity text, not split remarks.");
}

}  // namespace

auto RunConvertIngestValidateTests(TestState& state) -> void {
  TestConvertResponses(state);
  TestIngestResponses(state);
  TestValidateResponses(state);
  TestContinuationDayPreservesFirstSegment(state);
  TestPointWithoutContextProducesEndOnly(state);
  TestEndOnlyBoundaryCanBeInheritedAcrossAdjacentDay(state);
  TestIntervalDayBuildsExplicitRecords(state);
  TestMixedDayUsesIntervalEndAsNextPointBoundary(state);
  TestContinuationDayAllowsGapBeforeFirstInterval(state);
  TestWrappedIntervalPreservesCrossMidnightDuration(state);
  TestCrossMidnightIntervalDoesNotUseWrapThreshold(state);
  TestPureIntervalTxtParsesSparseDaysWithoutContext(state);
  TestMixedTxtUsesIntervalEndAndLeavesGapsUnrecorded(state);
  TestMixedTxtContiguousSampleUsesSharedTimeline(state);
  TestMixedTxtPointAfterCrossMidnightIntervalUsesExpandedBoundary(state);
  TestMultilineRemarksUsePhysicalContinuationLines(state);
  TestHashAndSemicolonAreNotRemarkDelimiters(state);
}

}  // namespace tracer_core::application::tests
