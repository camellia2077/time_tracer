# Canonical Text Contract v1

## Scope

This contract applies to runtime-managed text artifacts:

1. `TXT` ingest/export files
2. runtime/config `TOML` files
3. tracer exchange text entries:
   - `manifest.toml`
   - payload `*.txt`
   - packaged converter `*.toml`

## Write Contract

All producers MUST write canonical text bytes:

1. encoding: `UTF-8`
2. BOM: forbidden
3. newline: `LF` only

No platform-specific `includeBom` or legacy write mode is allowed.

## Read Contract

All text readers MUST normalize input at the outer boundary before parse,
validate, or business logic:

1. decode as `UTF-8` only
2. reject malformed UTF-8 with an explicit error
3. strip exactly one leading `UTF-8 BOM` when present
4. normalize `CRLF` and bare `CR` to `LF`

No downstream parser or validator should implement its own BOM/newline
compatibility path.

## Validation Contract

Validation operates on canonical text only.

1. validator/parser inputs are already normalized
2. validation failures should report the normalized-text error cause
3. top-level error messages should keep the concrete text failure visible

## Exchange Contract

Tracer exchange packaging does not preserve legacy text bytes.

1. export canonicalizes text entries before packaging
2. import canonicalizes known text entries before writing to disk
3. identical semantic text exported from different hosts should produce
   identical canonical text entry bytes

## Out Of Scope

This contract does not define Unicode NFC/NFD normalization yet.

1. current requirement is valid `UTF-8` plus canonical BOM/newline handling
2. Unicode normalization may be added later at the same boundary
