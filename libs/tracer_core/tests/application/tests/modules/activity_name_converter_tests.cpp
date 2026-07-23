import tracer.core.application.activity_name_converter;
import tracer.core.domain.types.converter_config;

#include "application/tests/modules/pipeline_tests.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>

namespace tracer_core::application::tests {

using tracer::core::application::modconverter::ActivityNameMappingDirection;
using tracer::core::application::modconverter::ActivityNameTextConverter;
using tracer::core::domain::types::ConverterConfig;

namespace {

auto BuildRepoRoot() -> std::filesystem::path {
  return std::filesystem::path(__FILE__)
      .parent_path()   // modules
      .parent_path()   // tests
      .parent_path()   // application
      .parent_path()   // tests
      .parent_path()   // tracer_core
      .parent_path()   // libs
      .parent_path();  // repo root
}

auto ReadFixtureText(const std::filesystem::path& relative_path)
    -> std::string {
  const std::filesystem::path fixture_path = BuildRepoRoot() / relative_path;
  std::ifstream input(fixture_path, std::ios::binary);
  if (!input.is_open()) {
    return {};
  }
  return std::string(std::istreambuf_iterator<char>(input),
                     std::istreambuf_iterator<char>());
}

auto BuildTestConfig() -> ConverterConfig {
  ConverterConfig config;
  config.text_mapping["英语单词"] = "study_english_words";
  config.text_mapping["english_words_alias"] = "study_english_words";
  config.text_mapping["英语写作"] = "study_english_writing";
  config.sleep_inference.wake_keywords = {"wake"};
  return config;
}

auto TestNameConversionIsIdempotent(TestState& state) -> void {
  const ActivityNameTextConverter converter(BuildTestConfig());

  Expect(state,
         converter.ConvertName(
             "英语单词", ActivityNameMappingDirection::kAliasToCanonical) ==
             "study_english_words",
         "Alias-to-canonical conversion should resolve a configured alias.");
  Expect(
      state,
      converter.ConvertName("study_english_words",
                            ActivityNameMappingDirection::kAliasToCanonical) ==
          "study_english_words",
      "Alias-to-canonical conversion should preserve canonical input.");
  Expect(
      state,
      converter.ConvertName("english_words_alias",
                            ActivityNameMappingDirection::kCanonicalToAlias) ==
          "english_words_alias",
      "Canonical-to-alias conversion should preserve alias input.");
  Expect(
      state,
      converter.ConvertName("study_english_words",
                            ActivityNameMappingDirection::kCanonicalToAlias) ==
          "english_words_alias",
      "Canonical-to-alias conversion should select a stable alias.");
  Expect(
      state,
      converter.ConvertName("unknown_activity",
                            ActivityNameMappingDirection::kAliasToCanonical) ==
          "unknown_activity",
      "Unknown activity names should remain unchanged.");
  Expect(
      state,
      converter.ConvertName(
          "wake", ActivityNameMappingDirection::kAliasToCanonical) == "wake" &&
          converter.ConvertName(
              "wake", ActivityNameMappingDirection::kCanonicalToAlias) ==
              "wake",
      "Wake keywords should remain structural TXT tokens.");
}

auto TestSingleMonthFixture(TestState& state) -> void {
  const std::string aliases =
      ReadFixtureText("test/fixtures/text/alias_mapping/2026-01.aliases.txt");
  Expect(state, !aliases.empty(),
         "Single-month alias fixture should be readable.");
  if (aliases.empty()) {
    return;
  }

  const ActivityNameTextConverter converter(BuildTestConfig());
  const std::string canonical = converter.ConvertText(
      aliases, ActivityNameMappingDirection::kAliasToCanonical);
  const std::string expected =
      "y2026\nm01\n\n"
      "d0103\n"
      "0700wake\n"
      "0830study_english_words // alias event\n"
      "1000study_english_writing // another alias\n"
      "1130-1200study_english_words // interval alias\n";
  Expect(state, canonical == expected,
         "Single-month conversion should change only activity names.");
  Expect(state,
         converter.ConvertText(
             canonical, ActivityNameMappingDirection::kAliasToCanonical) ==
             canonical,
         "Alias-to-canonical conversion should be idempotent for a month TXT.");
}

auto TestMultiMonthFixture(TestState& state) -> void {
  const std::string january =
      ReadFixtureText("test/fixtures/text/alias_mapping/2026-01.aliases.txt");
  const std::string february =
      ReadFixtureText("test/fixtures/text/alias_mapping/2026-02.canonical.txt");
  Expect(state, !january.empty() && !february.empty(),
         "Multi-month mapping fixtures should be readable.");
  if (january.empty() || february.empty()) {
    return;
  }

  const ActivityNameTextConverter converter(BuildTestConfig());
  const std::string combined = january + february;
  const std::string canonical = converter.ConvertText(
      combined, ActivityNameMappingDirection::kAliasToCanonical);
  const std::string expected =
      "y2026\nm01\n\n"
      "d0103\n"
      "0700wake\n"
      "0830study_english_words // alias event\n"
      "1000study_english_writing // another alias\n"
      "1130-1200study_english_words // interval alias\n"
      "y2026\nm02\n\n"
      "d0201\n"
      "0700wake\n"
      "0830study_english_words // canonical event\n"
      "1000study_english_writing // another canonical event\n"
      "1130-1200study_english_words // canonical interval\n";
  Expect(state, canonical == expected,
         "Multi-month conversion should preserve month boundaries.");

  const std::string aliases = converter.ConvertText(
      canonical, ActivityNameMappingDirection::kCanonicalToAlias);
  const std::string expected_aliases =
      "y2026\nm01\n\n"
      "d0103\n"
      "0700wake\n"
      "0830english_words_alias // alias event\n"
      "1000英语写作 // another alias\n"
      "1130-1200english_words_alias // interval alias\n"
      "y2026\nm02\n\n"
      "d0201\n"
      "0700wake\n"
      "0830english_words_alias // canonical event\n"
      "1000英语写作 // another canonical event\n"
      "1130-1200english_words_alias // canonical interval\n";
  Expect(state, aliases == expected_aliases,
         "Multi-month reverse conversion should be stable across files.");
  Expect(
      state,
      converter.ConvertText(
          aliases, ActivityNameMappingDirection::kCanonicalToAlias) == aliases,
      "Canonical-to-alias conversion should be idempotent for multi-month "
      "TXT.");
}

auto TestCanonicalReplacementIsExact(TestState& state) -> void {
  const ActivityNameTextConverter converter(BuildTestConfig());
  const std::unordered_map<std::string, std::string> replacements = {
      {"exercise_walk", "exercise_cardio_walk"},
  };
  const std::string source =
      "y2026\nm01\n\nd0103\n"
      "0830exercise_walk // exercise_walk remark\n"
      "0900english_words_alias\n"
      "1000-1030exercise_walk\n";
  const std::string expected =
      "y2026\nm01\n\nd0103\n"
      "0830exercise_cardio_walk // exercise_walk remark\n"
      "0900english_words_alias\n"
      "1000-1030exercise_cardio_walk\n";
  Expect(state,
         converter.ReplaceCanonicalNames(source, replacements) == expected,
         "Canonical replacement should change only matching event names.");
}

}  // namespace

auto RunActivityNameConverterTests(TestState& state) -> void {
  TestNameConversionIsIdempotent(state);
  TestSingleMonthFixture(state);
  TestMultiMonthFixture(state);
  TestCanonicalReplacementIsExact(state);
}

}  // namespace tracer_core::application::tests
