# Validation Capability

This directory is the primary home for `tracer_core` validation docs.

Use it when you need to understand:
1. what validation owns
2. the required validation order
3. the difference between config validation, TXT structure validation, and TXT
   logic validation
4. how canonical text normalization and diagnostics fit into the flow

## Read First
1. [overview.md](overview.md)
   - Capability boundary, responsibilities, and source-of-truth rules.
2. [sop.md](sop.md)
   - Required validation order and the persistence gate rule.
3. [txt_structure.md](txt_structure.md)
   - Header, line, and state-machine validation for TXT input.
4. [txt_logic.md](txt_logic.md)
   - Semantic rules that depend on config and parsed activity meaning.
5. [config_validation.md](config_validation.md)
   - Converter `TOML` validation and how it gates downstream TXT checks.
6. [canonical_text.md](canonical_text.md)
   - Shared canonical text normalization rules as used by validation.
7. [diagnostics.md](diagnostics.md)
   - Error-code and diagnostic shaping at validation outputs.

## Validation Layers
1. Config validation
2. Canonical text normalization
3. TXT structure validation
4. TXT logic validation
5. Persistence gate handoff to ingest

## Not Owned Here
1. SQLite write-side repository construction
2. Query/report rendering behavior
3. Host-side UI or CLI argument parsing

## Legacy Pointers
Validation topics were previously spread across:
1. `core/design/ingest-persistence-boundary.md`
2. `core/contracts/text/canonical_text_contract_v1.md`
3. `core/contracts/error-codes.md`
4. `core/ingest/day_bucket_and_wake_anchor_semantics.md`
