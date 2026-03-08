import tracer.transport;

#include <iostream>
#include <string_view>

namespace {

using tracer::transport::modenvelope::Build;
using tracer::transport::modenvelope::Parse;
using tracer::transport::modenvelope::Serialize;
using tracer::transport::moderrors::Code;
using tracer::transport::moderrors::Make;
using tracer::transport::modfields::BuildTypeError;
using tracer::transport::modfields::FormatFieldIssue;

auto Contains(std::string_view text, std::string_view pattern) -> bool {
  return text.find(pattern) != std::string_view::npos;
}

auto Expect(bool condition, std::string_view message, int& failures) -> void {
  if (condition) {
    return;
  }
  ++failures;
  std::cerr << "[FAIL] " << message << '\n';
}

void TestErrorBridge(int& failures) {
  const auto error = Make(Code::kParseFailure, "bad payload");
  Expect(error.HasError(), "Make should return non-empty error.", failures);
  Expect(error.code == Code::kParseFailure, "Error code mismatch.", failures);
  Expect(error.message == "bad payload", "Error message mismatch.", failures);
}

void TestEnvelopeBridge(int& failures) {
  const auto envelope = Build(true, "", "module smoke payload");
  const auto serialized = Serialize(envelope);
  const auto parsed = Parse(serialized, "module_smoke");
  Expect(!parsed.HasError(), "Round-trip parse should succeed.", failures);
  Expect(parsed.envelope.ok, "Round-trip `ok` should be true.", failures);
  Expect(parsed.envelope.content == "module smoke payload",
         "Round-trip content mismatch.", failures);

  const auto bad = Parse("{", "module_smoke");
  Expect(bad.HasError(), "Invalid JSON should fail parsing.", failures);
  Expect(bad.error.code == Code::kParseFailure,
         "Invalid JSON should return parse failure.", failures);
  Expect(Contains(bad.error.message, "module_smoke:"),
         "Invalid JSON error should include context.", failures);
}

void TestFieldBridge(int& failures) {
  const auto issue = BuildTypeError("limit", "integer");
  const auto formatted = FormatFieldIssue(issue);
  Expect(issue.field_name == "limit", "Field bridge should preserve name.",
         failures);
  Expect(Contains(formatted, "limit:"), "Formatted field issue mismatch.",
         failures);
}

}  // namespace

auto main() -> int {
  int failures = 0;
  TestErrorBridge(failures);
  TestEnvelopeBridge(failures);
  TestFieldBridge(failures);

  if (failures == 0) {
    std::cout << "[PASS] tracer_transport_modules_smoke_tests\n";
    return 0;
  }

  std::cerr << "[FAIL] tracer_transport_modules_smoke_tests failures: "
            << failures << '\n';
  return 1;
}
