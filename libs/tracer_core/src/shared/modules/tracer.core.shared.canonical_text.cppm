module;

#include "shared/utils/canonical_text.hpp"

export module tracer.core.shared.canonical_text;

export namespace tracer::core::shared::canonical_text {

using ::tracer::core::shared::canonical_text::CanonicalizationResult;
using ::tracer::core::shared::canonical_text::Canonicalize;
using ::tracer::core::shared::canonical_text::RequireCanonicalText;
using ::tracer::core::shared::canonical_text::ToUtf8Bytes;

}  // namespace tracer::core::shared::canonical_text

export namespace tracer::core::shared::modtext {

using tracer::core::shared::canonical_text::CanonicalizationResult;
using tracer::core::shared::canonical_text::Canonicalize;
using tracer::core::shared::canonical_text::RequireCanonicalText;
using tracer::core::shared::canonical_text::ToUtf8Bytes;

}  // namespace tracer::core::shared::modtext
