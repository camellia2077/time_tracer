# Validation And Canonical Text

Validation consumes canonical text only.

The shared cross-capability contract lives in:
1. [../../shared/canonical_text_contract_v1.md](../../shared/canonical_text_contract_v1.md)

This page explains why that contract matters specifically for validation.

## Why Validation Needs Canonical Text

Without a single normalization boundary, downstream validators would each need
their own compatibility logic for:
1. `UTF-8 BOM`
2. `CRLF` vs `LF`
3. malformed `UTF-8`

That would make diagnostics inconsistent and would allow hidden host drift.

## Validation Expectations

Before parse or validate:
1. input has been decoded as `UTF-8`
2. malformed `UTF-8` has already failed explicitly
3. at most one leading BOM has been stripped
4. newlines have been normalized to `LF`

After that point:
1. parsers and validators operate on normalized text
2. diagnostics refer to normalized-text semantics
3. exchange/import/export can rely on the same text contract

## Why This Is Shared, Not Validation-Only

The same canonical text rule also constrains:
1. ingest
2. exchange packaging/import
3. runtime-managed `TOML` config

So the normative byte-level contract remains under `shared/`, while validation
documents how it is consumed.
