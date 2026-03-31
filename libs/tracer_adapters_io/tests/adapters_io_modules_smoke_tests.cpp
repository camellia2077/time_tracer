import tracer.adapters.io;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.model.time_data_models;

#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#ifndef TT_ENABLE_PROCESSED_JSON_IO
#define TT_ENABLE_PROCESSED_JSON_IO 1
#endif

namespace {

namespace fs = std::filesystem;

using tracer::adapters::io::modcore::CreateDirectories;
using tracer::adapters::io::modcore::Exists;
using tracer::adapters::io::modcore::IsDirectory;
using tracer::adapters::io::modcore::IsRegularFile;
using tracer::adapters::io::modcore::ReadBytes;
using tracer::adapters::io::modcore::ReadCanonicalText;
using tracer::adapters::io::modcore::WriteBytes;
using tracer::adapters::io::modcore::WriteCanonicalText;
using tracer::adapters::io::modruntime::CreateProcessedDataLoader;
using tracer::adapters::io::modruntime::CreateProcessedDataStorage;
using tracer::adapters::io::modruntime::CreateTxtIngestInputProvider;
using tracer::adapters::io::modutils::FindFilesByExtensionRecursively;
using tracer::adapters::io::modutils::ResolveFiles;
using tracer::core::domain::modmodel::BaseActivityRecord;
using tracer::core::domain::modmodel::DailyLog;

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

auto MakeWorkspaceRoot() -> fs::path {
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return fs::temp_directory_path() /
         ("tt_adapters_io_modules_smoke_" + std::to_string(now));
}

auto BuildRoundTripDay() -> DailyLog {
  DailyLog day;
  day.date = "2026-03-15";
  day.getupTime = "07:00";
  day.hasStudyActivity = true;

  BaseActivityRecord first;
  first.logical_id = 1;
  first.start_timestamp = 25200;
  first.end_timestamp = 28800;
  first.start_time_str = "07:00";
  first.end_time_str = "08:00";
  first.project_path = "study_cpp";
  first.duration_seconds = 3600;

  BaseActivityRecord second;
  second.logical_id = 2;
  second.start_timestamp = 28800;
  second.end_timestamp = 32400;
  second.start_time_str = "08:00";
  second.end_time_str = "09:00";
  second.project_path = "study_cpp.modules";
  second.duration_seconds = 3600;

  day.processedActivities.push_back(first);
  day.processedActivities.push_back(second);
  day.activityCount = static_cast<int>(day.processedActivities.size());
  return day;
}

void TestCoreAndUtilsBridge(int& failures) {
  const fs::path root = MakeWorkspaceRoot();
  const fs::path nested = root / "nested";
  const fs::path note = nested / "note.txt";
  const fs::path second = nested / "more.txt";
  const fs::path ignored = nested / "ignore.log";

  try {
    CreateDirectories(nested);
    WriteCanonicalText(note, "hello modules");
    WriteCanonicalText(second, "second line");
    WriteCanonicalText(ignored, "ignored");

    Expect(Exists(root), "Root directory should exist.", failures);
    Expect(IsDirectory(nested), "Nested path should be directory.", failures);
    Expect(IsRegularFile(note), "note.txt should be a regular file.", failures);
    Expect(ReadCanonicalText(note) == "hello modules",
           "ReadCanonicalText result mismatch.", failures);
    const auto note_bytes = ReadBytes(note);
    Expect(note_bytes == std::vector<std::uint8_t>{'h', 'e', 'l', 'l', 'o', ' ',
                                                   'm', 'o', 'd', 'u', 'l', 'e',
                                                   's'},
           "ReadBytes should preserve canonical UTF-8 payload bytes.",
           failures);

    const auto files = FindFilesByExtensionRecursively(root, ".txt");
    Expect(files.size() == 2U, "FindFilesByExtensionRecursively size mismatch.",
           failures);

    const std::vector<std::string> input_paths = {root.string(), note.string()};
    const auto resolved = ResolveFiles(input_paths, ".txt");
    Expect(resolved.size() == 2U, "ResolveFiles should deduplicate entries.",
           failures);
  } catch (const std::exception& ex) {
    ++failures;
    std::cerr << "[FAIL] Unexpected exception: " << ex.what() << '\n';
  }

  std::error_code ec;
  (void)fs::remove_all(root, ec);
}

void TestRuntimeFactories(int& failures) {
  const fs::path root = MakeWorkspaceRoot();
  const fs::path note = root / "runtime.txt";
  const fs::path output_root = root / "output";

  try {
    CreateDirectories(root);
    const std::vector<std::uint8_t> legacy_bytes = {
        0xEFU, 0xBBU, 0xBFU, 'r', 'u', 'n', 't', 'i', 'm',
        'e',   '\r',  '\n',  'f', 'a', 'c', 't', 'o', 'r',
        'y',   '\r',  'b',   'r', 'i', 'd', 'g', 'e'};
    WriteBytes(note, legacy_bytes);

    auto ingest_input_provider = CreateTxtIngestInputProvider();
    auto processed_data_loader = CreateProcessedDataLoader();
    auto processed_data_storage = CreateProcessedDataStorage();

    Expect(static_cast<bool>(ingest_input_provider),
           "CreateTxtIngestInputProvider should return a port instance.",
           failures);
    Expect(static_cast<bool>(processed_data_loader),
           "CreateProcessedDataLoader should return a port instance.",
           failures);
    Expect(static_cast<bool>(processed_data_storage),
           "CreateProcessedDataStorage should return a port instance.",
           failures);

    const auto collected =
        ingest_input_provider->CollectTextInputs(root, ".txt");
    Expect(collected.input_exists,
           "Runtime factory provider should observe existing input root.",
           failures);
    Expect(collected.inputs.size() == 1U,
           "Runtime factory provider should collect txt payloads.", failures);
    Expect(collected.inputs.front().content == "runtime\nfactory\nbridge",
           "Runtime factory provider should canonicalize UTF-8 input content.",
           failures);

    const std::map<std::string, std::vector<DailyLog>> data = {
        {"2026-03", {BuildRoundTripDay()}}};
    const auto written =
        processed_data_storage->WriteProcessedData(data, output_root);

#if TT_ENABLE_PROCESSED_JSON_IO
    Expect(written.size() == 1U,
           "Processed-data storage should emit one monthly JSON file.",
           failures);
    if (!written.empty()) {
      Expect(Exists(written.front()),
             "Processed-data storage should create the JSON file.", failures);
      const auto written_text = ReadCanonicalText(written.front());
      const auto written_json = nlohmann::json::parse(written_text);
      const bool has_status =
          written_json.is_array() && !written_json.empty() &&
          written_json.front().contains("headers") &&
          written_json.front()["headers"].contains("status");
      const bool has_exercise =
          written_json.is_array() && !written_json.empty() &&
          written_json.front().contains("headers") &&
          written_json.front()["headers"].contains("exercise");
      Expect(!has_status,
             "Processed-data JSON should not persist derived header.status.",
             failures);
      Expect(!has_exercise,
             "Processed-data JSON should not persist derived header.exercise.",
             failures);
      const bool has_wake_anchor =
          written_json.is_array() && !written_json.empty() &&
          written_json.front().contains("headers") &&
          written_json.front()["headers"].contains("wake_anchor");
      Expect(
          has_wake_anchor &&
              written_json.front()["headers"]["wake_anchor"] == 1,
          "Processed-data JSON should persist header.wake_anchor from day semantics.",
          failures);
    }

    const auto loaded =
        processed_data_loader->LoadDailyLogs((output_root / "data").string());
    Expect(loaded.errors.empty(),
           "Processed-data loader should accept module-written JSON files.",
           failures);
    Expect(loaded.data_by_source.size() == 1U,
           "Processed-data loader should load one source file.", failures);
    if (!loaded.data_by_source.empty()) {
      const auto& loaded_days = loaded.data_by_source.begin()->second;
      Expect(loaded_days.size() == 1U,
             "Processed-data loader should deserialize one day.", failures);
      if (!loaded_days.empty()) {
        Expect(loaded_days.front().date == "2026-03-15",
               "Processed-data loader should preserve the day date.", failures);
        Expect(loaded_days.front().processedActivities.size() == 2U,
               "Processed-data loader should preserve activity rows.",
               failures);
      }
    }

    const fs::path legacy_root = root / "legacy_input";
    CreateDirectories(legacy_root);
    const fs::path legacy_file = legacy_root / "2026-03.json";
    WriteCanonicalText(legacy_file,
                       R"([
  {
    "headers": {
      "date": "2026-03-16",
      "status": 0,
      "exercise": 0,
      "wake_anchor": 1,
      "getup": "07:30",
      "activity_count": 2,
      "remark": "legacy"
    },
    "activities": [
      {
        "logical_id": 1,
        "start_timestamp": 25200,
        "end_timestamp": 27000,
        "start_time": "07:00",
        "end_time": "07:30",
        "duration_seconds": 1800,
        "activity_remark": null,
        "activity": { "project_path": "study_cpp" }
      },
      {
        "logical_id": 2,
        "start_timestamp": 27000,
        "end_timestamp": 28800,
        "start_time": "07:30",
        "end_time": "08:00",
        "duration_seconds": 1800,
        "activity_remark": null,
        "activity": { "project_path": "exercise_cardio" }
      }
    ]
  }
])");
    const auto legacy_loaded =
        processed_data_loader->LoadDailyLogs(legacy_root.string());
    Expect(legacy_loaded.errors.empty(),
           "Processed-data loader should accept legacy header.status/exercise "
           "fields.",
           failures);
    Expect(legacy_loaded.data_by_source.size() == 1U,
           "Processed-data loader should load one legacy source file.",
           failures);
    if (!legacy_loaded.data_by_source.empty()) {
      const auto& legacy_days = legacy_loaded.data_by_source.begin()->second;
      Expect(!legacy_days.empty(),
             "Processed-data loader should deserialize legacy day rows.",
             failures);
      if (!legacy_days.empty()) {
        Expect(legacy_days.front().hasStudyActivity,
               "Legacy JSON load should rebuild study flag from activities.",
               failures);
        Expect(legacy_days.front().hasExerciseActivity,
               "Legacy JSON load should rebuild exercise flag from activities.",
               failures);
        Expect(legacy_days.front().hasWakeAnchor,
               "Legacy JSON load should rebuild wake anchor from "
               "getup/isContinuation.",
               failures);
      }
    }

    const fs::path sparse_root = root / "sparse_input";
    CreateDirectories(sparse_root);
    const fs::path sparse_file = sparse_root / "2026-04.json";
    WriteCanonicalText(sparse_file,
                       R"([
  {
    "headers": {
      "date": "2026-04-01",
      "status": 0,
      "exercise": 0,
      "wake_anchor": 1,
      "getup": "07:30",
      "activity_count": 1,
      "remark": ""
    },
    "activities": [
      {
        "logical_id": 1,
        "start_timestamp": 25200,
        "end_timestamp": 27000,
        "start_time": "07:00",
        "end_time": "07:30",
        "duration_seconds": 1800,
        "activity_remark": null,
        "activity": { "project_path": "study_cpp" }
      }
    ]
  }
])");
    const auto sparse_loaded =
        processed_data_loader->LoadDailyLogs(sparse_root.string());
    Expect(sparse_loaded.errors.empty(),
           "Processed-data loader should accept JSON days with one activity.",
           failures);
    Expect(sparse_loaded.data_by_source.size() == 1U,
           "Processed-data loader should load sparse source files.",
           failures);
    if (!sparse_loaded.data_by_source.empty()) {
      const auto& sparse_days = sparse_loaded.data_by_source.begin()->second;
      Expect(sparse_days.size() == 1U,
             "Processed-data loader should deserialize sparse day rows.",
             failures);
      if (!sparse_days.empty()) {
        Expect(sparse_days.front().processedActivities.size() == 1U,
               "Processed-data loader should preserve sparse activity rows.",
               failures);
      }
    }
#else
    Expect(written.empty(),
           "Processed-data storage should no-op when processed JSON I/O is "
           "disabled.",
           failures);

    const auto loaded =
        processed_data_loader->LoadDailyLogs((output_root / "data").string());
    Expect(loaded.data_by_source.empty(),
           "Processed-data loader should no-op when processed JSON I/O is "
           "disabled.",
           failures);
    Expect(loaded.errors.empty(),
           "Processed-data loader no-op path should not fabricate validation "
           "errors.",
           failures);
#endif
  } catch (const std::exception& ex) {
    ++failures;
    std::cerr << "[FAIL] Unexpected exception: " << ex.what() << '\n';
  }

  std::error_code ec;
  (void)fs::remove_all(root, ec);
}

void TestRuntimeFactoriesRejectInvalidUtf8(int& failures) {
  const fs::path root = MakeWorkspaceRoot();
  const fs::path note = root / "broken.txt";

  try {
    CreateDirectories(root);
    const std::vector<std::uint8_t> invalid_bytes = {0xFFU, 'x', '\n'};
    WriteBytes(note, invalid_bytes);

    auto ingest_input_provider = CreateTxtIngestInputProvider();
    bool threw = false;
    std::string message;
    try {
      static_cast<void>(ingest_input_provider->CollectTextInputs(root, ".txt"));
    } catch (const std::exception& exception) {
      threw = true;
      message = exception.what();
    }

    Expect(threw, "Txt ingest provider should reject invalid UTF-8 files.",
           failures);
    Expect(message.find("Invalid UTF-8") != std::string::npos,
           "Txt ingest provider failure should mention invalid UTF-8.",
           failures);
  } catch (const std::exception& ex) {
    ++failures;
    std::cerr << "[FAIL] Unexpected exception: " << ex.what() << '\n';
  }

  std::error_code ec;
  (void)fs::remove_all(root, ec);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestCoreAndUtilsBridge(failures);
  TestRuntimeFactories(failures);
  TestRuntimeFactoriesRejectInvalidUtf8(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_adapters_io_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_adapters_io_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
