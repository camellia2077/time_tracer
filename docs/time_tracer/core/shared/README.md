# Core Shared Docs

This directory holds cross-capability contracts and semantics that are shared by
multiple core capabilities.

## Put Docs Here When
1. The document is consumed by more than one capability.
2. The document defines a stable shared contract rather than a single workflow.
3. The document should stay independent from a single capability owner folder.

## Current Docs
1. [canonical_text_contract_v1.md](canonical_text_contract_v1.md)
   - Canonical UTF-8, BOM, and newline contract used by validation, ingest,
     exchange, and runtime-managed text IO.
2. [error-codes.md](error-codes.md)
   - Shared error-code vocabulary, with validation-related mappings linked from
     the validation capability docs.
3. [error-model.md](error-model.md)
   - Shared error and logging model across runtime hosts.
4. [c_abi.md](c_abi.md)
   - Stable `tracer_core_*` ABI naming and JSON-boundary contract.
