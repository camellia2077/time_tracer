#ifndef APPLICATION_ACTIVITY_NAME_CONVERTER_H_
#define APPLICATION_ACTIVITY_NAME_CONVERTER_H_

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

struct ConverterConfig;

enum class ActivityNameMappingDirection {
  kAliasToCanonical,
  kCanonicalToAlias,
};

// Converts activity names without changing the surrounding TXT structure.
//
// Canonical-to-alias conversion is intentionally deterministic: when several
// aliases point to the same canonical name, the lexicographically smallest
// alias is selected. Names that already belong to the requested representation
// are returned unchanged. Wake keywords are structural TXT tokens, so they are
// never rewritten by this converter.
class ActivityNameTextConverter {
 public:
  explicit ActivityNameTextConverter(const ConverterConfig& config);

  [[nodiscard]] auto ConvertName(std::string_view name,
                                 ActivityNameMappingDirection direction) const
      -> std::string;

  // Converts event-line activity names in a canonical UTF-8 TXT document.
  // Headers, blank lines, general remarks, event times, and inline remarks are
  // preserved. CRLF input is normalized to LF by the shared text contract.
  [[nodiscard]] auto ConvertText(std::string_view text,
                                 ActivityNameMappingDirection direction) const
      -> std::string;

  // Replaces only the supplied canonical activity paths. This is used by
  // hierarchy migrations, where aliases and all other user-authored names
  // must remain byte-for-byte untouched.
  [[nodiscard]] auto ReplaceCanonicalNames(
      std::string_view text,
      const std::unordered_map<std::string, std::string>& replacements) const
      -> std::string;

  // Replaces only supplied authored alias tokens in event names. This is used
  // when an alias is renamed and historical TXT must remain parseable under
  // the updated alias document.
  [[nodiscard]] auto ReplaceAliasNames(
      std::string_view text,
      const std::unordered_map<std::string, std::string>& replacements) const
      -> std::string;

 private:
  [[nodiscard]] auto ConvertEventLine(
      std::string_view line, ActivityNameMappingDirection direction) const
      -> std::string;
  [[nodiscard]] auto ReplaceCanonicalNamesInEventLine(
      std::string_view line,
      const std::unordered_map<std::string, std::string>& replacements) const
      -> std::string;

  std::unordered_map<std::string, std::string> alias_to_canonical_;
  std::unordered_map<std::string, std::string> canonical_to_alias_;
  std::unordered_set<std::string> aliases_;
  std::unordered_set<std::string> canonical_names_;
  std::unordered_set<std::string> wake_keywords_;
  std::unordered_set<std::string> wake_canonical_names_;
};

#endif  // APPLICATION_ACTIVITY_NAME_CONVERTER_H_
