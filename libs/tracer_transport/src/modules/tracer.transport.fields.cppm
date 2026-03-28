module;

#include "tracer/transport/fields.hpp"

export module tracer.transport.fields;

export namespace tracer::transport::modfields {

using ::tracer::transport::BoolFieldResult;
using ::tracer::transport::BuildTypeError;
using ::tracer::transport::FieldIssue;
using ::tracer::transport::FormatFieldIssue;
using ::tracer::transport::IntFieldResult;
using ::tracer::transport::IntListFieldResult;
using ::tracer::transport::RequireStringField;
using ::tracer::transport::StringListFieldResult;
using ::tracer::transport::StringFieldResult;
using ::tracer::transport::TryReadBoolField;
using ::tracer::transport::TryReadIntField;
using ::tracer::transport::TryReadIntListField;
using ::tracer::transport::TryReadStringListField;
using ::tracer::transport::TryReadStringField;

}  // namespace tracer::transport::modfields
