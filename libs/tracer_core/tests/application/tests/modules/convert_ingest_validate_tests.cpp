import tracer.core.domain.logic.converter.core;
import tracer.core.domain.logic.converter.log_processor;
import tracer.core.domain.types.converter_config;

// application/tests/modules/convert_ingest_validate_tests.cpp
#include "application/tests/modules/pipeline_tests.hpp"
#include "application/tests/support/fakes.hpp"
#include "application/tests/support/test_support.hpp"

namespace tracer_core::application::tests {

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
  config.wake_keywords = {"wake"};
  return config;
}

auto TestConvertResponses(TestState& state) -> void {
  FakePipelineWorkflow pipeline_workflow;
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

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
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

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
  FakeReportHandler report_handler;
  auto runtime_api = BuildRuntimeApiForTest(pipeline_workflow, report_handler);

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
         "RunRecordActivityAtomically should succeed when time_order_mode is omitted.");
  Expect(state,
         pipeline_workflow.last_record_activity_request.time_order_mode ==
             TimeOrderMode::kStrictCalendar,
         "RunRecordActivityAtomically should default time_order_mode to strict_calendar.");

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
         "RunRecordActivityAtomically should return failed DTO when handler throws.");
  Expect(state,
         Contains(kAtomicRecordFailure.message,
                  "RunRecordActivityAtomically failed"),
         "RunRecordActivityAtomically failure should include operation name.");
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
  interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("0900"),
               .endTimeStr = "1030",
               .description = "study"});
  interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("1401"),
               .endTimeStr = "1900",
               .description = "sleep"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 2,
         "Pure interval day should produce one record per explicit interval.");
  if (interval_day.processedActivities.size() != 2) {
    return;
  }

  const auto& first = interval_day.processedActivities[0];
  const auto& second = interval_day.processedActivities[1];
  Expect(state, first.start_time_str == "09:00" && first.end_time_str == "10:30",
         "First explicit interval should keep its authored start/end.");
  Expect(state, second.start_time_str == "14:01" &&
                    second.end_time_str == "19:00",
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
  config.wake_keywords = {"wake"};
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-03";

  DailyLog mixed_day;
  mixed_day.date = "2026-02-04";
  mixed_day.getupTime = "06:06";
  mixed_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Point,
               .endTimeStr = "0606",
               .description = "wake"});
  mixed_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("0900"),
               .endTimeStr = "1030",
               .description = "study"});
  mixed_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Point,
               .endTimeStr = "1353",
               .description = "sleep"});

  processor.Process(previous_day, mixed_day);

  Expect(state, mixed_day.processedActivities.size() == 2,
         "Mixed day should emit one explicit interval and one derived point segment.");
  if (mixed_day.processedActivities.size() != 2) {
    return;
  }

  const auto& trailing_sleep = mixed_day.processedActivities.back();
  Expect(state, trailing_sleep.start_time_str == "10:30",
         "Point event after interval should start from the interval end boundary.");
  Expect(state, trailing_sleep.end_time_str == "13:53",
         "Point event after interval should end at the point authored time.");
}

auto TestContinuationDayAllowsGapBeforeFirstInterval(TestState& state) -> void {
  ConverterConfig config;
  config.text_mapping["study"] = "study";
  DayProcessor processor(config);

  DailyLog previous_day;
  previous_day.date = "2026-02-04";
  previous_day.rawEvents.push_back(RawEvent{.endTimeStr = "23:30"});

  DailyLog continuation_day;
  continuation_day.date = "2026-02-05";
  continuation_day.isContinuation = true;
  continuation_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("0900"),
               .endTimeStr = "1030",
               .description = "study"});

  processor.Process(previous_day, continuation_day);

  Expect(state, continuation_day.processedActivities.size() == 1,
         "Continuation day with first interval should keep the explicit interval.");
  if (continuation_day.processedActivities.empty()) {
    return;
  }

  const auto& first_activity = continuation_day.processedActivities.front();
  Expect(state, first_activity.start_time_str == "09:00" &&
                    first_activity.end_time_str == "10:30",
         "Continuation interval should not be rewritten to the previous-day boundary.");
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
  interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("2132"),
               .endTimeStr = "0135",
               .description = "study"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 1,
         "Wrapped interval should still produce one explicit record.");
  if (interval_day.processedActivities.empty()) {
    return;
  }

  const auto& activity = interval_day.processedActivities.front();
  Expect(state, activity.start_time_str == "21:32",
         "Wrapped interval should preserve the authored start boundary.");
  Expect(state, activity.end_time_str == "01:35",
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
  interval_day.rawEvents.push_back(
      RawEvent{.kind = RawEventKindType::Interval,
               .startTimeStr = std::string("1030"),
               .endTimeStr = "0900",
               .description = "study",
               .remark = "@allow-long"});

  processor.Process(previous_day, interval_day);

  Expect(state, interval_day.processedActivities.size() == 1,
         "Cross-midnight interval should produce one explicit record.");
  if (interval_day.processedActivities.empty()) {
    return;
  }

  const auto& activity = interval_day.processedActivities.front();
  Expect(state, activity.start_time_str == "10:30",
         "Cross-midnight interval should preserve the authored start boundary.");
  Expect(state, activity.end_time_str == "09:00",
         "Cross-midnight interval should preserve the authored end boundary.");
  Expect(state, activity.duration_seconds == ((22 * 60) + 30) * 60,
         "Cross-midnight interval should be interpreted as ending next day.");
  Expect(state, activity.remark.has_value() &&
                    activity.remark.value().find("@allow-long") !=
                        std::string::npos,
         "Cross-midnight interval should preserve @allow-long remark.");
}

auto TestPureIntervalTxtParsesSparseDaysWithoutContext(TestState& state)
    -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent(
      "interval-month.txt",
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
         "First interval event should not mark the day as overnight continuation.");
  if (march_fifth.processedActivities.empty()) {
    return;
  }

  const auto& first_record = march_fifth.processedActivities.front();
  Expect(state, first_record.start_time_str == "08:03" &&
                    first_record.end_time_str == "09:07",
         "Sparse pure interval day should use its own explicit first range.");
}

auto TestMixedTxtUsesIntervalEndAndLeavesGapsUnrecorded(TestState& state)
    -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent(
      "mixed-month.txt",
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
  Expect(state, day.processedActivities.size() == 4,
         "Mixed day should emit only recorded segments and leave gaps empty.");
  if (day.processedActivities.size() != 4) {
    return;
  }

  const auto& game = day.processedActivities[0];
  const auto& sleep = day.processedActivities[1];
  const auto& study = day.processedActivities[2];
  const auto& internet = day.processedActivities[3];
  Expect(state, game.start_time_str == "08:09" &&
                    game.end_time_str == "12:00" &&
                    game.project_path == "game",
         "Point event should use the previous point boundary.");
  Expect(state, sleep.start_time_str == "12:30" &&
                    sleep.end_time_str == "13:04",
         "Interval event should keep its explicit range after a point gap.");
  Expect(state, study.start_time_str == "14:04" &&
                    study.end_time_str == "16:23",
         "Later interval event should keep its explicit range after a gap.");
  Expect(state, internet.start_time_str == "16:23" &&
                    internet.end_time_str == "18:09",
         "Point event after interval should start at the interval end.");
}

auto TestMixedTxtContiguousSampleUsesSharedTimeline(TestState& state) -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent(
      "mixed-contiguous-month.txt",
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
  Expect(state, day.processedActivities.size() == 4,
         "Contiguous mixed day should emit the four materialized activities.");
  if (day.processedActivities.size() != 4) {
    return;
  }

  const auto& game = day.processedActivities[0];
  const auto& sleep = day.processedActivities[1];
  const auto& study = day.processedActivities[2];
  const auto& internet = day.processedActivities[3];
  Expect(state, game.project_path == "game" &&
                    game.start_time_str == "08:09" &&
                    game.end_time_str == "12:00",
         "First materialized point activity should use the previous point boundary.");
  Expect(state, sleep.project_path == "sleep" &&
                    sleep.start_time_str == "12:00" &&
                    sleep.end_time_str == "13:04",
         "Contiguous interval should keep its explicit start and end.");
  Expect(state, study.project_path == "study" &&
                    study.start_time_str == "13:04" &&
                    study.end_time_str == "16:23",
         "Following contiguous interval should keep its explicit start and end.");
  Expect(state, internet.project_path == "internet" &&
                    internet.start_time_str == "16:23" &&
                    internet.end_time_str == "18:09",
         "Point event after contiguous intervals should use the last interval end.");
}

auto TestMixedTxtPointAfterCrossMidnightIntervalUsesExpandedBoundary(
    TestState& state) -> void {
  ConverterConfig config = BuildTimelineTestConfig();
  LogProcessor processor(config);

  const auto result = processor.ProcessSourceContent(
      "mixed-cross-midnight-month.txt",
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
         "Cross-midnight mixed day should emit interval and trailing point activity.");
  if (day.processedActivities.size() != 2) {
    return;
  }

  const auto& study = day.processedActivities[0];
  const auto& game = day.processedActivities[1];
  Expect(state, study.project_path == "study" &&
                    study.start_time_str == "21:32" &&
                    study.end_time_str == "01:35" &&
                    study.duration_seconds == ((4 * 60) + 3) * 60,
         "Cross-midnight interval should keep next-day duration.");
  Expect(state, game.project_path == "game" &&
                    game.start_time_str == "01:35" &&
                    game.end_time_str == "23:50" &&
                    game.duration_seconds == ((22 * 60) + 15) * 60,
         "Point after cross-midnight interval should use the expanded previous end boundary.");
}

}  // namespace

auto RunConvertIngestValidateTests(TestState& state) -> void {
  TestConvertResponses(state);
  TestIngestResponses(state);
  TestValidateResponses(state);
  TestContinuationDayPreservesFirstSegment(state);
  TestIntervalDayBuildsExplicitRecords(state);
  TestMixedDayUsesIntervalEndAsNextPointBoundary(state);
  TestContinuationDayAllowsGapBeforeFirstInterval(state);
  TestWrappedIntervalPreservesCrossMidnightDuration(state);
  TestCrossMidnightIntervalDoesNotUseWrapThreshold(state);
  TestPureIntervalTxtParsesSparseDaysWithoutContext(state);
  TestMixedTxtUsesIntervalEndAndLeavesGapsUnrecorded(state);
  TestMixedTxtContiguousSampleUsesSharedTimeline(state);
  TestMixedTxtPointAfterCrossMidnightIntervalUsesExpandedBoundary(state);
}

}  // namespace tracer_core::application::tests
