import tracer.adapters.io;
import tracer.core.domain.model.daily_log;
import tracer.core.domain.model.time_data_models;

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

using tracer::core::domain::modmodel::BaseActivityRecord;
using tracer::core::domain::modmodel::DailyLog;
using tracer::adapters::io::modcore::CreateDirectories;
using tracer::adapters::io::modcore::Exists;
using tracer::adapters::io::modcore::IsDirectory;
using tracer::adapters::io::modcore::IsRegularFile;
using tracer::adapters::io::modcore::ReadContent;
using tracer::adapters::io::modcore::WriteContent;
using tracer::adapters::io::modruntime::CreateProcessedDataLoader;
using tracer::adapters::io::modruntime::CreateProcessedDataStorage;
using tracer::adapters::io::modruntime::CreateTxtIngestInputProvider;
using tracer::adapters::io::modutils::FindFilesByExtensionRecursively;
using tracer::adapters::io::modutils::ResolveFiles;

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
  day.stats.study_time = 7200;
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
    WriteContent(note, "hello modules");
    WriteContent(second, "second line");
    WriteContent(ignored, "ignored");

    Expect(Exists(root), "Root directory should exist.", failures);
    Expect(IsDirectory(nested), "Nested path should be directory.", failures);
    Expect(IsRegularFile(note), "note.txt should be a regular file.", failures);
    Expect(ReadContent(note) == "hello modules", "ReadContent result mismatch.",
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
    WriteContent(note, "runtime factory bridge");

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
    Expect(collected.inputs.front().content == "runtime factory bridge",
           "Runtime factory provider should read file content.", failures);

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
#else
    Expect(written.empty(),
           "Processed-data storage should no-op when processed JSON I/O is disabled.",
           failures);

    const auto loaded =
        processed_data_loader->LoadDailyLogs((output_root / "data").string());
    Expect(loaded.data_by_source.empty(),
           "Processed-data loader should no-op when processed JSON I/O is disabled.",
           failures);
    Expect(loaded.errors.empty(),
           "Processed-data loader no-op path should not fabricate validation errors.",
           failures);
#endif
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

  if (failures == 0) {
    std::cout << "[PASS] tracer_adapters_io_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_adapters_io_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
